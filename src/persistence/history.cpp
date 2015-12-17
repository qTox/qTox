#include "history.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/persistence/historykeeper.h"
#include <QDebug>
#include <cassert>

using namespace std;

History::History(const QString &profileName, const QString &password)
    : db{getDbPath(profileName), password}
{
    init();
}

History::History(const QString &profileName, const QString &password, const HistoryKeeper &oldHistory)
    : History{profileName, password}
{
    import(oldHistory);
}

History::~History()
{
    // We could have execLater requests pending with a lambda attached,
    // so clear the pending transactions first
    db.sync();
}

bool History::isValid()
{
    return db.isOpen();
}

void History::setPassword(const QString& password)
{
    db.setPassword(password);
}

void History::rename(const QString &newName)
{
    db.rename(getDbPath(newName));
}

void History::eraseHistory()
{
    db.execNow("DELETE FROM faux_offline_pending;"
               "DELETE FROM history;"
               "DELETE FROM aliases;"
               "DELETE FROM peers;"
               "VACUUM;");
}

void History::removeFriendHistory(const QString &friendPk)
{
    if (!peers.contains(friendPk))
        return;
    int64_t id = peers[friendPk];

    if (db.execNow(QString("DELETE FROM faux_offline_pending "
               "WHERE faux_offline_pending.id IN ( "
                 "SELECT faux_offline_pending.id FROM faux_offline_pending "
                 "LEFT JOIN history ON faux_offline_pending.id = history.id "
                 "WHERE chat_id=%1 "
               "); "
               "DELETE FROM history WHERE chat_id=%1; "
               "DELETE FROM aliases WHERE owner=%1; "
               "DELETE FROM peers WHERE id=%1; "
               "VACUUM;").arg(id)))
    {
        peers.remove(friendPk);
    }
    else
    {
        qWarning() << "Failed to remove friend's history";
    }
}

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
            peerId = *max_element(begin(peers), end(peers))+1;
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
            senderId = *max_element(begin(peers), end(peers))+1;
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

void History::addNewMessage(const QString &friendPk, const QString &message, const QString &sender,
                const QDateTime &time, bool isSent, QString dispName, std::function<void(int64_t)> insertIdCallback)
{
    db.execLater(generateNewMessageQueries(friendPk, message, sender, time, isSent, dispName, insertIdCallback));
}

QList<History::HistMessage> History::getChatHistory(const QString &friendPk, const QDateTime &from, const QDateTime &to)
{
    QList<HistMessage> messages;

    auto rowCallback = [&messages](const QVector<QVariant>& row)
    {
        messages += {row[0].toLongLong(),
                    row[1].isNull(),
                    QDateTime::fromMSecsSinceEpoch(row[2].toLongLong()),
                    row[3].toString(),
                    row[4].toString(),
                    row[5].toString(),
                    row[6].toString()};
    };

    // Don't forget to update the rowCallback if you change the selected columns!
    db.execNow({QString("SELECT history.id, faux_offline_pending.id, timestamp, chat.public_key, "
                               "aliases.display_name, sender.public_key, message FROM history "
                       "LEFT JOIN faux_offline_pending ON history.id = faux_offline_pending.id "
                       "JOIN peers chat ON chat_id = chat.id "
                       "JOIN aliases ON sender_alias = aliases.id "
                       "JOIN peers sender ON aliases.owner = sender.id "
                       "WHERE timestamp BETWEEN %1 AND %2 AND chat.public_key='%3';")
                        .arg(from.toMSecsSinceEpoch()).arg(to.toMSecsSinceEpoch()).arg(friendPk), rowCallback});

    return messages;
}

void History::markAsSent(qint64 id)
{
    db.execLater(QString("DELETE FROM faux_offline_pending WHERE id=%1;").arg(id));
}

QString History::getDbPath(const QString &profileName)
{
    return Settings::getInstance().getSettingsDirPath() + profileName + ".db";
}

void History::init()
{
    if (!isValid())
    {
        qWarning() << "Database not open, init failed";
        return;
    }

    db.execLater("CREATE TABLE IF NOT EXISTS peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE);"
                 "CREATE TABLE IF NOT EXISTS aliases (id INTEGER PRIMARY KEY, owner INTEGER,"
                                                     "display_name BLOB NOT NULL, UNIQUE(owner, display_name));"
                 "CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, "
                                                     "chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, "
                                                     "message BLOB NOT NULL);"
                 "CREATE TABLE IF NOT EXISTS faux_offline_pending (id INTEGER PRIMARY KEY);");

    // Cache our current peers
    db.execLater(RawDatabase::Query{"SELECT id, public_key FROM peers;", [this](const QVector<QVariant>& row)
    {
        peers[row[1].toString()] = row[0].toInt();
    }});
}

void History::import(const HistoryKeeper &oldHistory)
{
    if (!isValid())
    {
        qWarning() << "New database not open, import failed";
        return;
    }

    qDebug() << "Importing old database...";
    QTime t=QTime::currentTime();
    t.start();
    QVector<RawDatabase::Query> queries;
    constexpr int batchSize = 1000;
    queries.reserve(batchSize);
    QList<HistoryKeeper::HistMessage> oldMessages = oldHistory.exportMessagesDeleteFile();
    for (const HistoryKeeper::HistMessage& msg : oldMessages)
    {
        queries += generateNewMessageQueries(msg.chat, msg.message, msg.sender, msg.timestamp, true, msg.dispName);
        if (queries.size() == batchSize)
        {
            db.execLater(queries);
            queries.clear();
        }
    }
    db.execLater(queries);
    db.sync();
    qDebug() << "Imported old database in"<<t.elapsed()<<"ms";
}
