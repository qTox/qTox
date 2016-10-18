#include "history.h"

#include <QDebug>
#include <algorithm>

#include "historykeeper.h"

/**
 * @class History
 * @brief Interacts with the user info database to save the chat history.
 *
 * @var QHash<QString, int64_t> History::peers
 * @brief Maps friend public keys to unique IDs by index.
 * Caches mappings to speed up message saving.
 */

/**
 * @brief Init History by database
 * @param db Database with user information
 */
History::History(UserDb* db)
    : db(db)
{
    // Cache our current peers
    db->execLater(RawDatabase::Query("SELECT public_key, id FROM peers;",
                                 [this](const QVector<QVariant>& row)
    {
        peers[row[0].toString()] = row[1].toInt();
    }));
}

/**
 * @brief Imports messages from the old history file.
 * @param oldHistory Old history to import.
 */
void History::import(const HistoryKeeper &oldHistory)
{
    if (!db->isOpen())
    {
        qWarning() << "New database not open, import failed";
        return;
    }

    qDebug() << "Importing old database...";
    QTime t = QTime::currentTime();
    t.start();
    QVector<RawDatabase::Query> queries;
    constexpr int batchSize = 1000;
    queries.reserve(batchSize);
    QList<HistoryKeeper::HistMessage> oldMessages = oldHistory.exportMessagesDeleteFile();
    for (const HistoryKeeper::HistMessage& msg : oldMessages)
    {
        queries += generateNewMessageQueries(msg.chat, msg.message, msg.sender,
                                             msg.timestamp, true, msg.dispName);
        if (queries.size() >= batchSize)
        {
            db->execLater(queries);
            queries.clear();
        }
    }
    db->execLater(queries);
    db->sync();
    qDebug() << "Imported old database in" << t.elapsed() << "ms";
}

/**
 * @brief Erases all the chat history from the database.
 */
void History::eraseHistory()
{
    db->execNow("DELETE FROM faux_offline_pending;"
                "DELETE FROM history;"
                "DELETE FROM aliases;"
                "DELETE FROM peers;"
                "VACUUM;");
}

/**
 * @brief Erases the chat history with one friend.
 * @param friendPk Friend public key to erase.
 */
void History::removeFriendHistory(const QString &friendPk)
{
    if (!peers.contains(friendPk))
        return;

    int64_t id = peers[friendPk];

    bool deleted = db->execNow(QString(
               "DELETE FROM faux_offline_pending "
               "WHERE faux_offline_pending.id IN ( "
               "     SELECT faux_offline_pending.id FROM faux_offline_pending "
               "     LEFT JOIN history ON faux_offline_pending.id = history.id "
               "     WHERE chat_id=%1 "
               "); "
               "DELETE FROM history WHERE chat_id=%1; "
               "DELETE FROM aliases WHERE owner=%1; "
               "DELETE FROM peers WHERE id=%1; "
               "VACUUM;").arg(id));

    if (deleted)
        peers.remove(friendPk);
    else
        qWarning() << "Failed to remove friend's history";
}

/**
 * @brief Generate query to insert new message in database
 * @param friendPk Friend publick key to save.
 * @param message Message to save.
 * @param sender Sender to save.
 * @param time Time of message sending.
 * @param isSent True if message was already sent.
 * @param dispName Name, which should be displayed.
 * @param insertIdCallback Function, called after query execution.
 */
QVector<RawDatabase::Query> History::generateNewMessageQueries(const QString &friendPk, const QString &message,
                                        const QString &sender, const QDateTime &time, bool isSent, QString dispName,
                                                               std::function<void(int64_t)> insertIdCallback)
{
    QVector<RawDatabase::Query> queries;

    // Get the db id of the peer we're chatting with
    int64_t peerId;
    if (peers.contains(friendPk))
    {
        peerId = peers[friendPk];
    }
    else
    {
        if (peers.isEmpty())
            peerId = 0;
        else
            peerId = *std::max_element(peers.begin(), peers.end()) + 1;

        peers[friendPk] = peerId;
        queries += RawDatabase::Query{("INSERT INTO peers (id, public_key) VALUES (%1, '"+friendPk+"');").arg(peerId)};
    }

    // Get the db id of the sender of the message
    int64_t senderId;
    if (peers.contains(sender))
    {
        senderId = peers[sender];
    }
    else
    {
        if (peers.isEmpty())
            senderId = 0;
        else
            senderId = *std::max_element(peers.begin(), peers.end()) + 1;

        peers[sender] = senderId;
        queries += RawDatabase::Query{("INSERT INTO peers (id, public_key) VALUES (%1, '"+sender+"');").arg(senderId)};
    }

    queries += RawDatabase::Query(QString("INSERT OR IGNORE INTO aliases (owner, display_name) VALUES (%1, ?);")
                                  .arg(senderId), {dispName.toUtf8()});

    // If the alias already existed, the insert will ignore the conflict and last_insert_rowid() will return garbage,
    // so we have to check changes() and manually fetch the row ID in this case
    queries += RawDatabase::Query(QString("INSERT INTO history (timestamp, chat_id, message, sender_alias) "
                                          "VALUES (%1, %2, ?, ("
                                          "  CASE WHEN changes() IS 0 THEN ("
                                          "    SELECT id FROM aliases WHERE owner=%3 AND display_name=?)"
                                          "  ELSE last_insert_rowid() END"
                                          "));")
                                    .arg(time.toMSecsSinceEpoch()).arg(peerId).arg(senderId),
                                    {message.toUtf8(), dispName.toUtf8()}, insertIdCallback);

    if (!isSent)
        queries += RawDatabase::Query{"INSERT INTO faux_offline_pending (id) VALUES (last_insert_rowid());"};

    return queries;
}

/**
 * @brief Saves a chat message in the database.
 * @param friendPk Friend publick key to save.
 * @param message Message to save.
 * @param sender Sender to save.
 * @param time Time of message sending.
 * @param isSent True if message was already sent.
 * @param dispName Name, which should be displayed.
 * @param insertIdCallback Function, called after query execution.
 */
void History::addNewMessage(const QString &friendPk, const QString &message, const QString &sender,
                const QDateTime &time, bool isSent, QString dispName, std::function<void(int64_t)> insertIdCallback)
{
    db->execLater(generateNewMessageQueries(friendPk, message, sender, time, isSent, dispName, insertIdCallback));
}

/**
 * @brief Fetches chat messages from the database.
 * @param friendPk Friend publick key to fetch.
 * @param from Start of period to fetch.
 * @param to End of period to fetch.
 * @return List of messages.
 */
QList<History::HistMessage> History::getChatHistory(const QString &friendPk, const QDateTime &from, const QDateTime &to)
{
    QList<HistMessage> messages;

    auto rowCallback = [&messages](const QVector<QVariant>& row)
    {
        // dispName and message could have null bytes, QString::fromUtf8 truncates on null bytes so we strip them
        messages += {row[0].toLongLong(),
                    row[1].isNull(),
                    QDateTime::fromMSecsSinceEpoch(row[2].toLongLong()),
                    row[3].toString(),
                    QString::fromUtf8(row[4].toByteArray().replace('\0',"")),
                    row[5].toString(),
                    QString::fromUtf8(row[6].toByteArray().replace('\0',""))};
    };

    // Don't forget to update the rowCallback if you change the selected columns!
    db->execNow({QString("SELECT history.id, faux_offline_pending.id, timestamp, chat.public_key, "
                       "         aliases.display_name, sender.public_key, message FROM history "
                       "LEFT JOIN faux_offline_pending ON history.id = faux_offline_pending.id "
                       "JOIN peers chat ON chat_id = chat.id "
                       "JOIN aliases ON sender_alias = aliases.id "
                       "JOIN peers sender ON aliases.owner = sender.id "
                       "WHERE timestamp BETWEEN %1 AND %2 AND chat.public_key='%3';")
                        .arg(from.toMSecsSinceEpoch()).arg(to.toMSecsSinceEpoch()).arg(friendPk), rowCallback});

    return messages;
}

/**
 * @brief Marks a message as sent.
 * Removing message from the faux-offline pending messages list.
 *
 * @param id Message ID.
 */
void History::markAsSent(qint64 id)
{
    db->execLater(QString("DELETE FROM faux_offline_pending WHERE id=%1;").arg(id));
}
