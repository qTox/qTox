/*
    Copyright © 2015-2016 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include <cassert>

#include "db/rawdatabase.h"
#include "history.h"
#include "profile.h"
#include "settings.h"

/**
 * @class History
 * @brief Interacts with the profile database to save the chat history.
 *
 * @var QHash<QString, int64_t> History::peers
 * @brief Maps friend public keys to unique IDs by index.
 * Caches mappings to speed up message saving.
 */

/**
 * @brief Prepares the database to work with the history.
 * @param db This database will be prepared for use with the history.
 */
History::History(std::shared_ptr<RawDatabase> db)
    : db(db)
{
    if (!isValid())
    {
        qWarning() << "Database not open, init failed";
        return;
    }

    db->execLater("CREATE TABLE IF NOT EXISTS peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE);"
                 "CREATE TABLE IF NOT EXISTS aliases (id INTEGER PRIMARY KEY, owner INTEGER,"
                                                     "display_name BLOB NOT NULL, UNIQUE(owner, display_name));"
                 "CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, "
                                                     "chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, "
                                                     "message BLOB NOT NULL);"
                 "CREATE TABLE IF NOT EXISTS faux_offline_pending (id INTEGER PRIMARY KEY);");

    // Cache our current peers
    db->execLater(RawDatabase::Query{"SELECT public_key, id FROM peers;", [this](const QVector<QVariant>& row)
    {
        peers[row[0].toString()] = row[1].toInt();
    }});
}

History::~History()
{
    if (!isValid())
    {
        return;
    }

    // We could have execLater requests pending with a lambda attached,
    // so clear the pending transactions first
    db->sync();
}

/**
 * @brief Checks if the database was opened successfully
 * @return True if database if opened, false otherwise.
 */
bool History::isValid()
{
    return db && db->isOpen();
}

/**
 * @brief Erases all the chat history from the database.
 */
void History::eraseHistory()
{
    if (!isValid())
    {
        return;
    }

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
void History::removeFriendHistory(const QString& friendPk)
{
    if (!isValid())
    {
        return;
    }

    if (!peers.contains(friendPk))
    {
        return;
    }

    int64_t id = peers[friendPk];

    QString queryText = QString(
                "DELETE FROM faux_offline_pending "
                "WHERE faux_offline_pending.id IN ( "
                "    SELECT faux_offline_pending.id FROM faux_offline_pending "
                "    LEFT JOIN history ON faux_offline_pending.id = history.id "
                "    WHERE chat_id=%1 "
                "); "
                "DELETE FROM history WHERE chat_id=%1; "
                "DELETE FROM aliases WHERE owner=%1; "
                "DELETE FROM peers WHERE id=%1; "
                "VACUUM;").arg(id);

    if (db->execNow(queryText))
    {
        peers.remove(friendPk);
    }
    else
    {
        qWarning() << "Failed to remove friend's history";
    }
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
QVector<RawDatabase::Query> History::generateNewMessageQueries(const QString& friendPk, const QString& message,
                                        const QString& sender, const QDateTime& time, bool isSent, QString dispName,
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
        {
            peerId = 0;
        }
        else
        {
            peerId = *std::max_element(peers.begin(), peers.end()) + 1;
        }

        peers[friendPk] = peerId;
        queries += RawDatabase::Query(("INSERT INTO peers (id, public_key) "
                                       "VALUES (%1, '" + friendPk + "');")
                .arg(peerId));
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
        {
            senderId = 0;
        }
        else
        {
            senderId = *std::max_element(peers.begin(), peers.end()) + 1;
        }

        peers[sender] = senderId;
        queries += RawDatabase::Query{("INSERT INTO peers (id, public_key) "
                                       "VALUES (%1, '" + sender + "');")
                .arg(senderId)};
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
    {
        queries += RawDatabase::Query{
                "INSERT INTO faux_offline_pending (id) VALUES ("
                "    last_insert_rowid()"
                ");"};
    }

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
void History::addNewMessage(const QString& friendPk, const QString& message,
                            const QString& sender, const QDateTime& time,
                            bool isSent, QString dispName,
                            std::function<void(int64_t)> insertIdCallback)
{
    if (!isValid())
    {
        return;
    }

    db->execLater(generateNewMessageQueries(friendPk, message, sender, time,
                                            isSent, dispName, insertIdCallback));
}

/**
 * @brief Fetches chat messages from the database.
 * @param friendPk Friend publick key to fetch.
 * @param from Start of period to fetch.
 * @param to End of period to fetch.
 * @return List of messages.
 */
QList<History::HistMessage> History::getChatHistory(const QString& friendPk,
                                                    const QDateTime& from,
                                                    const QDateTime& to)
{
    if (!isValid())
    {
        return {};
    }

    QList<HistMessage> messages;

    auto rowCallback = [&messages](const QVector<QVariant>& row)
    {
        // dispName and message could have null bytes, QString::fromUtf8
        // truncates on null bytes so we strip them
        messages += {row[0].toLongLong(),
                    row[1].isNull(),
                    QDateTime::fromMSecsSinceEpoch(row[2].toLongLong()),
                    row[3].toString(),
                    QString::fromUtf8(row[4].toByteArray().replace('\0',"")),
                    row[5].toString(),
                    QString::fromUtf8(row[6].toByteArray().replace('\0',""))};
    };

    // Don't forget to update the rowCallback if you change the selected columns!
    QString queryText = QString(
                "SELECT history.id, faux_offline_pending.id, timestamp, "
                "chat.public_key, aliases.display_name, sender.public_key, "
                "message FROM history "
                "LEFT JOIN faux_offline_pending ON history.id = faux_offline_pending.id "
                "JOIN peers chat ON chat_id = chat.id "
                "JOIN aliases ON sender_alias = aliases.id "
                "JOIN peers sender ON aliases.owner = sender.id "
                "WHERE timestamp BETWEEN %1 AND %2 AND chat.public_key='%3';")
            .arg(from.toMSecsSinceEpoch()).arg(to.toMSecsSinceEpoch()).arg(friendPk);

    db->execNow({queryText, rowCallback});

    return messages;
}

/**
 * @brief Marks a message as sent.
 * Removing message from the faux-offline pending messages list.
 *
 * @param id Message ID.
 */
void History::markAsSent(qint64 messageId)
{
    if (!isValid())
    {
        return;
    }

    db->execLater(QString("DELETE FROM faux_offline_pending WHERE id=%1;")
                  .arg(messageId));
}
