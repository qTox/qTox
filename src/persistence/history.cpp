/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "history.h"
#include "profile.h"
#include "settings.h"
#include "db/rawdatabase.h"
#include "src/core/toxpk.h"

namespace {
static constexpr int SCHEMA_VERSION = 5;

bool createCurrentSchema(RawDatabase& db)
{
    QVector<RawDatabase::Query> queries;
    queries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE peers (id INTEGER PRIMARY KEY, "
        "public_key TEXT NOT NULL UNIQUE);"
        "CREATE TABLE aliases (id INTEGER PRIMARY KEY, "
        "owner INTEGER, "
        "display_name BLOB NOT NULL, "
        "UNIQUE(owner, display_name), "
        "FOREIGN KEY (owner) REFERENCES peers(id));"
        "CREATE TABLE history "
        "(id INTEGER PRIMARY KEY, "
        "timestamp INTEGER NOT NULL, "
        "chat_id INTEGER NOT NULL, "
        "sender_alias INTEGER NOT NULL, "
        // even though technically a message can be null for file transfer, we've opted
        // to just insert an empty string when there's no content, this moderately simplifies
        // implementation as currently our database doesn't have support for optional fields.
        // We would either have to insert "?" or "null" based on if message exists and then
        // ensure that our blob vector always has the right number of fields. Better to just
        // leave this as NOT NULL for now.
        "message BLOB NOT NULL, "
        "file_id INTEGER, "
        "FOREIGN KEY (file_id) REFERENCES file_transfers(id), "
        "FOREIGN KEY (chat_id) REFERENCES peers(id), "
        "FOREIGN KEY (sender_alias) REFERENCES aliases(id));"
        "CREATE TABLE file_transfers "
        "(id INTEGER PRIMARY KEY, "
        "chat_id INTEGER NOT NULL, "
        "file_restart_id BLOB NOT NULL, "
        "file_name BLOB NOT NULL, "
        "file_path BLOB NOT NULL, "
        "file_hash BLOB NOT NULL, "
        "file_size INTEGER NOT NULL, "
        "direction INTEGER NOT NULL, "
        "file_state INTEGER NOT NULL);"
        "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY, "
        "FOREIGN KEY (id) REFERENCES history(id));"
        "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY, "
        "FOREIGN KEY (id) REFERENCES history(id));"));
    // sqlite doesn't support including the index as part of the CREATE TABLE statement, so add a second query
    queries += RawDatabase::Query(
        "CREATE INDEX chat_id_idx on history (chat_id);");
    queries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = %1;").arg(SCHEMA_VERSION));
    return db.execNow(queries);
}

bool isNewDb(std::shared_ptr<RawDatabase>& db, bool& success)
{
    bool newDb;
    if (!db->execNow(RawDatabase::Query("SELECT COUNT(*) FROM sqlite_master;",
                                        [&](const QVector<QVariant>& row) {
                                            newDb = row[0].toLongLong() == 0;
                                        }))) {
        db.reset();
        success = false;
        return false;
    }
    success = true;
    return newDb;
}

bool dbSchema0to1(RawDatabase& db)
{
    QVector<RawDatabase::Query> queries;
    queries +=
        RawDatabase::Query(QStringLiteral(
            "CREATE TABLE file_transfers "
            "(id INTEGER PRIMARY KEY, "
            "chat_id INTEGER NOT NULL, "
            "file_restart_id BLOB NOT NULL, "
            "file_name BLOB NOT NULL, "
            "file_path BLOB NOT NULL, "
            "file_hash BLOB NOT NULL, "
            "file_size INTEGER NOT NULL, "
            "direction INTEGER NOT NULL, "
            "file_state INTEGER NOT NULL);"));
    queries +=
        RawDatabase::Query(QStringLiteral("ALTER TABLE history ADD file_id INTEGER;"));
    queries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 1;"));
    return db.execNow(queries);
}

bool dbSchema1to2(RawDatabase& db)
{
    // Any faux_offline_pending message, in a chat that has newer delivered
    // message is decided to be broken. It must be moved from
    // faux_offline_pending to broken_messages

    // the last non-pending message in each chat
    QString lastDeliveredQuery = QString(
        "SELECT chat_id, MAX(history.id) FROM "
        "history JOIN peers chat ON chat_id = chat.id "
        "LEFT JOIN faux_offline_pending ON history.id = faux_offline_pending.id "
        "WHERE faux_offline_pending.id IS NULL "
        "GROUP BY chat_id;");

    QVector<RawDatabase::Query> upgradeQueries;
    upgradeQueries +=
        RawDatabase::Query(QStringLiteral(
            "CREATE TABLE broken_messages "
            "(id INTEGER PRIMARY KEY);"));

    auto rowCallback = [&upgradeQueries](const QVector<QVariant>& row) {
        auto chatId = row[0].toLongLong();
        auto lastDeliveredHistoryId = row[1].toLongLong();

        upgradeQueries += QString("INSERT INTO broken_messages "
            "SELECT faux_offline_pending.id FROM "
            "history JOIN faux_offline_pending "
            "ON faux_offline_pending.id = history.id "
            "WHERE history.chat_id=%1 "
            "AND history.id < %2;").arg(chatId).arg(lastDeliveredHistoryId);
    };
    // note this doesn't modify the db, just generate new queries, so is safe
    // to run outside of our upgrade transaction
    if (!db.execNow({lastDeliveredQuery, rowCallback})) {
        return false;
    }

    upgradeQueries += QString(
        "DELETE FROM faux_offline_pending "
        "WHERE id in ("
            "SELECT id FROM broken_messages);");

    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 2;"));

    return db.execNow(upgradeQueries);
}

bool dbSchema2to3(RawDatabase& db)
{
    // Any faux_offline_pending message with the content "/me " are action
    // messages that qTox previously let a user enter, but that will cause an
    // action type message to be sent to toxcore, with 0 length, which will
    // always fail. They must be be moved from faux_offline_pending to broken_messages
    // to avoid qTox from erroring trying to send them on every connect

    const QString emptyActionMessageString = "/me ";

    QVector<RawDatabase::Query> upgradeQueries;
    upgradeQueries += RawDatabase::Query{QString("INSERT INTO broken_messages "
            "SELECT faux_offline_pending.id FROM "
            "history JOIN faux_offline_pending "
            "ON faux_offline_pending.id = history.id "
            "WHERE history.message = ?;"),
            {emptyActionMessageString.toUtf8()}};

    upgradeQueries += QString(
        "DELETE FROM faux_offline_pending "
        "WHERE id in ("
            "SELECT id FROM broken_messages);");

    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 3;"));

    return db.execNow(upgradeQueries);
}

bool dbSchema3to4(RawDatabase& db)
{
    QVector<RawDatabase::Query> upgradeQueries;
    upgradeQueries += RawDatabase::Query{QString(
        "CREATE INDEX chat_id_idx on history (chat_id);")};

    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 4;"));

    return db.execNow(upgradeQueries);
}

void addForeignKeyToAlias(QVector<RawDatabase::Query>& queries)
{
    queries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE aliases_new (id INTEGER PRIMARY KEY, owner INTEGER, "
        "display_name BLOB NOT NULL, UNIQUE(owner, display_name), "
        "FOREIGN KEY (owner) REFERENCES peers(id));"));
    queries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO aliases_new (id, owner, display_name) "
        "SELECT id, owner, display_name "
        "FROM aliases;"));
    queries += RawDatabase::Query(QStringLiteral("DROP TABLE aliases;"));
    queries += RawDatabase::Query(QStringLiteral("ALTER TABLE aliases_new RENAME TO aliases;"));
}

void addForeignKeyToHistory(QVector<RawDatabase::Query>& queries)
{
    queries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE history_new "
        "(id INTEGER PRIMARY KEY, "
        "timestamp INTEGER NOT NULL, "
        "chat_id INTEGER NOT NULL, "
        "sender_alias INTEGER NOT NULL, "
        "message BLOB NOT NULL, "
        "file_id INTEGER, "
        "FOREIGN KEY (file_id) REFERENCES file_transfers(id), "
        "FOREIGN KEY (chat_id) REFERENCES peers(id), "
        "FOREIGN KEY (sender_alias) REFERENCES aliases(id));"));
    queries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO history_new (id, timestamp, chat_id, sender_alias, message, file_id) "
        "SELECT id, timestamp, chat_id, sender_alias, message, file_id "
        "FROM history;"));
    queries += RawDatabase::Query(QStringLiteral("DROP TABLE history;"));
    queries += RawDatabase::Query(QStringLiteral("ALTER TABLE history_new RENAME TO history;"));
}

void addForeignKeyToFauxOfflinePending(QVector<RawDatabase::Query>& queries)
{
    queries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE new_faux_offline_pending (id INTEGER PRIMARY KEY, "
        "FOREIGN KEY (id) REFERENCES history(id));"));
    queries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO new_faux_offline_pending (id) "
        "SELECT id "
        "FROM faux_offline_pending;"));
    queries += RawDatabase::Query(QStringLiteral("DROP TABLE faux_offline_pending;"));
    queries += RawDatabase::Query(QStringLiteral("ALTER TABLE new_faux_offline_pending RENAME TO faux_offline_pending;"));
}

void addForeignKeyToBrokenMessages(QVector<RawDatabase::Query>& queries)
{
    queries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE new_broken_messages (id INTEGER PRIMARY KEY, "
        "FOREIGN KEY (id) REFERENCES history(id));"));
    queries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO new_broken_messages (id) "
        "SELECT id "
        "FROM broken_messages;"));
    queries += RawDatabase::Query(QStringLiteral("DROP TABLE broken_messages;"));
    queries += RawDatabase::Query(QStringLiteral("ALTER TABLE new_broken_messages RENAME TO broken_messages;"));
}

bool dbSchema4to5(RawDatabase& db)
{
    // add foreign key contrains to database tables. sqlite doesn't support advanced alter table commands, so instead we
    // need to copy data to new tables with the foreign key contraints: http://www.sqlitetutorial.net/sqlite-alter-table/
    QVector<RawDatabase::Query> upgradeQueries;
    addForeignKeyToAlias(upgradeQueries);
    addForeignKeyToHistory(upgradeQueries);
    addForeignKeyToFauxOfflinePending(upgradeQueries);
    addForeignKeyToBrokenMessages(upgradeQueries);
    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 5;"));
    auto transactionPass = db.execNow(upgradeQueries);
    if (transactionPass) {
        db.execNow("VACUUM;"); // after copying all the tables and deleting the old ones, our db file is half empty.
    }
    return transactionPass;
}

/**
* @brief Upgrade the db schema
* @return True if the schema upgrade succeded, false otherwise
* @note On future alterations of the database all you have to do is bump the SCHEMA_VERSION
* variable and add another case to the switch statement below. Make sure to fall through on each case.
*/
bool dbSchemaUpgrade(std::shared_ptr<RawDatabase>& db)
{
    int64_t databaseSchemaVersion;

    if (!db->execNow(RawDatabase::Query("PRAGMA user_version", [&](const QVector<QVariant>& row) {
            databaseSchemaVersion = row[0].toLongLong();
        }))) {
        qCritical() << "History failed to read user_version";
        return false;
    }

    if (databaseSchemaVersion > SCHEMA_VERSION) {
        qWarning().nospace() << "Database version (" << databaseSchemaVersion <<
            ") is newer than we currently support (" << SCHEMA_VERSION << "). Please upgrade qTox";
        // We don't know what future versions have done, we have to disable db access until we re-upgrade
        return false;
    } else if (databaseSchemaVersion == SCHEMA_VERSION) {
        // No work to do
        return true;
    }

    switch (databaseSchemaVersion) {
    case 0: {
        // Note: 0 is a special version that is actually two versions.
        //   possibility 1) it is a newly created database and it neesds the current schema to be created.
        //   possibility 2) it is a old existing database, before version 1 and before we saved schema version,
        //       and needs to be updated.
        bool success = false;
        const bool newDb = isNewDb(db, success);
        if (!success) {
            qCritical() << "Failed to create current db schema";
            return false;
        }
        if (newDb) {
            if (!createCurrentSchema(*db)) {
                qCritical() << "Failed to create current db schema";
                return false;
            }
            qDebug() << "Database created at schema version" << SCHEMA_VERSION;
            break; // new db is the only case where we don't incrementally upgrade through each version
        } else {
            if (!dbSchema0to1(*db)) {
                qCritical() << "Failed to upgrade db to schema version 1, aborting";
                return false;
            }
            qDebug() << "Database upgraded incrementally to schema version 1";
        }
    }
        // fallthrough
    case 1:
       if (!dbSchema1to2(*db)) {
            qCritical() << "Failed to upgrade db to schema version 2, aborting";
            return false;
       }
       qDebug() << "Database upgraded incrementally to schema version 2";
       //fallthrough
    case 2:
       if (!dbSchema2to3(*db)) {
            qCritical() << "Failed to upgrade db to schema version 3, aborting";
            return false;
       }
       qDebug() << "Database upgraded incrementally to schema version 3";
    case 3:
       if (!dbSchema3to4(*db)) {
            qCritical() << "Failed to upgrade db to schema version 4, aborting";
            return false;
       }
       qDebug() << "Database upgraded incrementally to schema version 4";
       //fallthrough
    case 4:
       if (!dbSchema4to5(*db)) {
            qCritical() << "Failed to upgrade db to schema version 5, aborting";
            return false;
       }
       qDebug() << "Database upgraded incrementally to schema version 5";
    // etc.
    default:
        qInfo() << "Database upgrade finished (databaseSchemaVersion" << databaseSchemaVersion
                << "->" << SCHEMA_VERSION << ")";
    }

    return true;
}

MessageState getMessageState(bool isPending, bool isBroken)
{
    assert(!(isPending && isBroken));
    MessageState messageState;
    if (isPending) {
        messageState = MessageState::pending;
    } else if (isBroken) {
        messageState = MessageState::broken;
    } else {
        messageState = MessageState::complete;
    }
    return messageState;
}
} // namespace

/**
 * @class History
 * @brief Interacts with the profile database to save the chat history.
 *
 * @var QHash<QString, int64_t> History::peers
 * @brief Maps friend public keys to unique IDs by index.
 * Caches mappings to speed up message saving.
 */

FileDbInsertionData::FileDbInsertionData()
{
    static int id = qRegisterMetaType<FileDbInsertionData>();
    (void)id;
}

/**
 * @brief Prepares the database to work with the history.
 * @param db This database will be prepared for use with the history.
 */
History::History(std::shared_ptr<RawDatabase> db_)
    : db(db_)
{
    if (!isValid()) {
        qWarning() << "Database not open, init failed";
        return;
    }

    // foreign key support is not enabled by default, so needs to be enabled on every connection
    // support was added in sqlite 3.6.19, which is qTox's minimum supported version
    db->execNow(
        "PRAGMA foreign_keys = ON;");

    const auto upgradeSucceeded = dbSchemaUpgrade(db);

    // dbSchemaUpgrade may have put us in an invalid state
    if (!upgradeSucceeded) {
        db.reset();
        return;
    }

    connect(this, &History::fileInsertionReady, this, &History::onFileInsertionReady);
    connect(this, &History::fileInserted, this, &History::onFileInserted);

    // Cache our current peers
    db->execLater(RawDatabase::Query{"SELECT public_key, id FROM peers;",
                                     [this](const QVector<QVariant>& row) {
                                         // HACK: we previously accidentally put Tox IDs in the db. So instead of
                                         // constructing as a ToxPk which will enforce the correct length, construct
                                         // as ToxId which will allow either length, and then convert to ToxPk.
                                         peers[ToxId{QByteArray::fromHex(row[0].toByteArray())}.getPublicKey()] = row[1].toInt();
                                     }});
}

History::~History()
{
    if (!isValid()) {
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
 * @brief Checks if a friend has chat history
 * @param friendPk
 * @return True if has, false otherwise.
 */
bool History::historyExists(const ToxPk& friendPk)
{
    if (historyAccessBlocked()) {
        return false;
    }

    return !getMessagesForFriend(friendPk, 0, 1).empty();
}

/**
 * @brief Erases all the chat history from the database.
 */
void History::eraseHistory()
{
    if (!isValid()) {
        return;
    }

    db->execNow("DELETE FROM faux_offline_pending;"
                "DELETE FROM broken_messages;"
                "DELETE FROM history;"
                "DELETE FROM aliases;"
                "DELETE FROM peers;"
                "DELETE FROM file_transfers;"
                "VACUUM;");
}

/**
 * @brief Erases the chat history with one friend.
 * @param friendPk Friend public key to erase.
 */
void History::removeFriendHistory(const ToxPk& friendPk)
{
    if (!isValid()) {
        return;
    }

    if (!peers.contains(friendPk)) {
        return;
    }

    int64_t id = peers[friendPk];

    QString queryText = QString("DELETE FROM faux_offline_pending "
                                "WHERE faux_offline_pending.id IN ( "
                                "    SELECT faux_offline_pending.id FROM faux_offline_pending "
                                "    LEFT JOIN history ON faux_offline_pending.id = history.id "
                                "    WHERE chat_id=%1 "
                                "); "
                                "DELETE FROM broken_messages "
                                "WHERE broken_messages.id IN ( "
                                "    SELECT broken_messages.id FROM broken_messages "
                                "    LEFT JOIN history ON broken_messages.id = history.id "
                                "    WHERE chat_id=%1 "
                                "); "
                                "DELETE FROM history WHERE chat_id=%1; "
                                "DELETE FROM aliases WHERE owner=%1; "
                                "DELETE FROM peers WHERE id=%1; "
                                "DELETE FROM file_transfers WHERE chat_id=%1;"
                                "VACUUM;")
                            .arg(id);

    if (db->execNow(queryText)) {
        peers.remove(friendPk);
    } else {
        qWarning() << "Failed to remove friend's history";
    }
}

/**
 * @brief Generate query to insert new message in database
 * @param friendPk Friend publick key to save.
 * @param message Message to save.
 * @param sender Sender to save.
 * @param time Time of message sending.
 * @param isDelivered True if message was already delivered.
 * @param dispName Name, which should be displayed.
 * @param insertIdCallback Function, called after query execution.
 */
QVector<RawDatabase::Query>
History::generateNewMessageQueries(const ToxPk& friendPk, const QString& message,
                                   const ToxPk& sender, const QDateTime& time, bool isDelivered,
                                   QString dispName, std::function<void(RowId)> insertIdCallback)
{
    QVector<RawDatabase::Query> queries;

    // Get the db id of the peer we're chatting with
    int64_t peerId;
    if (peers.contains(friendPk)) {
        peerId = (peers)[friendPk];
    } else {
        if (peers.isEmpty()) {
            peerId = 0;
        } else {
            peerId = *std::max_element(peers.begin(), peers.end()) + 1;
        }

        (peers)[friendPk] = peerId;
        queries += RawDatabase::Query(("INSERT INTO peers (id, public_key) "
                                       "VALUES (%1, '"
                                       + friendPk.toString() + "');")
                                          .arg(peerId));
    }

    // Get the db id of the sender of the message
    int64_t senderId;
    if (peers.contains(sender)) {
        senderId = (peers)[sender];
    } else {
        if (peers.isEmpty()) {
            senderId = 0;
        } else {
            senderId = *std::max_element(peers.begin(), peers.end()) + 1;
        }

        (peers)[sender] = senderId;
        queries += RawDatabase::Query{("INSERT INTO peers (id, public_key) "
                                       "VALUES (%1, '"
                                       + sender.toString() + "');")
                                          .arg(senderId)};
    }

    queries += RawDatabase::Query(
        QString("INSERT OR IGNORE INTO aliases (owner, display_name) VALUES (%1, ?);").arg(senderId),
        {dispName.toUtf8()});

    // If the alias already existed, the insert will ignore the conflict and last_insert_rowid()
    // will return garbage,
    // so we have to check changes() and manually fetch the row ID in this case
    queries +=
        RawDatabase::Query(QString(
                               "INSERT INTO history (timestamp, chat_id, message, sender_alias) "
                               "VALUES (%1, %2, ?, ("
                               "  CASE WHEN changes() IS 0 THEN ("
                               "    SELECT id FROM aliases WHERE owner=%3 AND display_name=?)"
                               "  ELSE last_insert_rowid() END"
                               "));")
                               .arg(time.toMSecsSinceEpoch())
                               .arg(peerId)
                               .arg(senderId),
                           {message.toUtf8(), dispName.toUtf8()}, insertIdCallback);

    if (!isDelivered) {
        queries += RawDatabase::Query{"INSERT INTO faux_offline_pending (id) VALUES ("
                                      "    last_insert_rowid()"
                                      ");"};
    }

    return queries;
}

void History::onFileInsertionReady(FileDbInsertionData data)
{

    QVector<RawDatabase::Query> queries;
    std::weak_ptr<History> weakThis = shared_from_this();

    // peerId is guaranteed to be inserted since we just used it in addNewMessage
    auto peerId = peers[data.friendPk];
    // Copy to pass into labmda for later
    auto fileId = data.fileId;
    queries +=
        RawDatabase::Query(QStringLiteral(
                               "INSERT INTO file_transfers (chat_id, file_restart_id, "
                               "file_path, file_name, file_hash, file_size, direction, file_state) "
                               "VALUES (%1, ?, ?, ?, ?, %2, %3, %4);")
                               .arg(peerId)
                               .arg(data.size)
                               .arg(static_cast<int>(data.direction))
                               .arg(ToxFile::CANCELED),
                           {data.fileId.toUtf8(), data.filePath.toUtf8(), data.fileName.toUtf8(), QByteArray()},
                           [weakThis, fileId](RowId id) {
                               auto pThis = weakThis.lock();
                               if (pThis) {
                                   emit pThis->fileInserted(id, fileId);
                               }
                           });


    queries += RawDatabase::Query(QStringLiteral("UPDATE history "
                                                 "SET file_id = (last_insert_rowid()) "
                                                 "WHERE id = %1")
                                      .arg(data.historyId.get()));

    db->execLater(queries);
}

void History::onFileInserted(RowId dbId, QString fileId)
{
    auto& fileInfo = fileInfos[fileId];
    if (fileInfo.finished) {
        db->execLater(
            generateFileFinished(dbId, fileInfo.success, fileInfo.filePath, fileInfo.fileHash));
        fileInfos.remove(fileId);
    } else {
        fileInfo.finished = false;
        fileInfo.fileId = dbId;
    }
}

RawDatabase::Query History::generateFileFinished(RowId id, bool success, const QString& filePath,
                                                 const QByteArray& fileHash)
{
    auto file_state = success ? ToxFile::FINISHED : ToxFile::CANCELED;
    if (filePath.length()) {
        return RawDatabase::Query(QStringLiteral("UPDATE file_transfers "
                                                 "SET file_state = %1, file_path = ?, file_hash = ?"
                                                 "WHERE id = %2")
                                      .arg(file_state)
                                      .arg(id.get()),
                                  {filePath.toUtf8(), fileHash});
    } else {
        return RawDatabase::Query(QStringLiteral("UPDATE file_transfers "
                                                 "SET finished = %1 "
                                                 "WHERE id = %2")
                                      .arg(file_state)
                                      .arg(id.get()));
    }
}

void History::addNewFileMessage(const ToxPk& friendPk, const QString& fileId,
                                const QString& fileName, const QString& filePath, int64_t size,
                                const ToxPk& sender, const QDateTime& time, QString const& dispName)
{
    if (historyAccessBlocked()) {
        return;
    }

    // This is an incredibly far from an optimal way of implementing this,
    // but given the frequency that people are going to be initiating a file
    // transfer we can probably live with it.

    // Since both inserting an alias for a user and inserting a file transfer
    // will generate new ids, there is no good way to inject both new ids into the
    // history query without refactoring our RawDatabase::Query and processor loops.

    // What we will do instead is chain callbacks to try to get reasonable behavior.
    // We can call the generateNewMessageQueries() fn to insert a message with an empty
    // message in it, and get the id with the callbck. Once we have the id we can ammend
    // the data to have our newly inserted file_id as well

    ToxFile::FileDirection direction;
    if (sender == friendPk) {
        direction = ToxFile::RECEIVING;
    } else {
        direction = ToxFile::SENDING;
    }

    std::weak_ptr<History> weakThis = shared_from_this();
    FileDbInsertionData insertionData;
    insertionData.friendPk = friendPk;
    insertionData.fileId = fileId;
    insertionData.fileName = fileName;
    insertionData.filePath = filePath;
    insertionData.size = size;
    insertionData.direction = direction;

    auto insertFileTransferFn = [weakThis, insertionData](RowId messageId) {
        auto insertionDataRw = std::move(insertionData);

        insertionDataRw.historyId = messageId;

        auto thisPtr = weakThis.lock();
        if (thisPtr)
            emit thisPtr->fileInsertionReady(std::move(insertionDataRw));
    };

    addNewMessage(friendPk, "", sender, time, true, dispName, insertFileTransferFn);
}

/**
 * @brief Saves a chat message in the database.
 * @param friendPk Friend publick key to save.
 * @param message Message to save.
 * @param sender Sender to save.
 * @param time Time of message sending.
 * @param isDelivered True if message was already delivered.
 * @param dispName Name, which should be displayed.
 * @param insertIdCallback Function, called after query execution.
 */
void History::addNewMessage(const ToxPk& friendPk, const QString& message, const ToxPk& sender,
                            const QDateTime& time, bool isDelivered, QString dispName,
                            const std::function<void(RowId)>& insertIdCallback)
{
    if (historyAccessBlocked()) {
        return;
    }

    db->execLater(generateNewMessageQueries(friendPk, message, sender, time, isDelivered, dispName,
                                            insertIdCallback));
}

void History::setFileFinished(const QString& fileId, bool success, const QString& filePath,
                              const QByteArray& fileHash)
{
    if (historyAccessBlocked()) {
        return;
    }

    auto& fileInfo = fileInfos[fileId];
    if (fileInfo.fileId.get() == -1) {
        fileInfo.finished = true;
        fileInfo.success = success;
        fileInfo.filePath = filePath;
        fileInfo.fileHash = fileHash;
    } else {
        db->execLater(generateFileFinished(fileInfo.fileId, success, filePath, fileHash));
    }

    fileInfos.remove(fileId);
}

size_t History::getNumMessagesForFriend(const ToxPk& friendPk)
{
    if (historyAccessBlocked()) {
        return 0;
    }

    return getNumMessagesForFriendBeforeDate(friendPk, QDateTime());
}

size_t History::getNumMessagesForFriendBeforeDate(const ToxPk& friendPk, const QDateTime& date)
{
    if (historyAccessBlocked()) {
        return 0;
    }

    QString queryText = QString("SELECT COUNT(history.id) "
                                "FROM history "
                                "JOIN peers chat ON chat_id = chat.id "
                                "WHERE chat.public_key='%1'")
                            .arg(friendPk.toString());

    if (date.isNull()) {
        queryText += ";";
    } else {
        queryText += QString(" AND timestamp < %1;").arg(date.toMSecsSinceEpoch());
    }

    size_t numMessages = 0;
    auto rowCallback = [&numMessages](const QVector<QVariant>& row) {
        numMessages = row[0].toLongLong();
    };

    db->execNow({queryText, rowCallback});

    return numMessages;
}

QList<History::HistMessage> History::getMessagesForFriend(const ToxPk& friendPk, size_t firstIdx,
                                                          size_t lastIdx)
{
    if (historyAccessBlocked()) {
        return {};
    }

    QList<HistMessage> messages;

    // Don't forget to update the rowCallback if you change the selected columns!
    QString queryText =
        QString("SELECT history.id, faux_offline_pending.id, timestamp, "
                "chat.public_key, aliases.display_name, sender.public_key, "
                "message, file_transfers.file_restart_id, "
                "file_transfers.file_path, file_transfers.file_name, "
                "file_transfers.file_size, file_transfers.direction, "
                "file_transfers.file_state, broken_messages.id FROM history "
                "LEFT JOIN faux_offline_pending ON history.id = faux_offline_pending.id "
                "JOIN peers chat ON history.chat_id = chat.id "
                "JOIN aliases ON sender_alias = aliases.id "
                "JOIN peers sender ON aliases.owner = sender.id "
                "LEFT JOIN file_transfers ON history.file_id = file_transfers.id "
                "LEFT JOIN broken_messages ON history.id = broken_messages.id "
                "WHERE chat.public_key='%1' "
                "LIMIT %2 OFFSET %3;")
            .arg(friendPk.toString())
            .arg(lastIdx - firstIdx)
            .arg(firstIdx);

    auto rowCallback = [&messages](const QVector<QVariant>& row) {
        // dispName and message could have null bytes, QString::fromUtf8
        // truncates on null bytes so we strip them
        auto id = RowId{row[0].toLongLong()};
        auto isPending = !row[1].isNull();
        auto timestamp = QDateTime::fromMSecsSinceEpoch(row[2].toLongLong());
        auto friend_key = row[3].toString();
        auto display_name = QString::fromUtf8(row[4].toByteArray().replace('\0', ""));
        auto sender_key = row[5].toString();
        auto isBroken = !row[13].isNull();

        MessageState messageState = getMessageState(isPending, isBroken);

        if (row[7].isNull()) {
            messages += {id, messageState, timestamp, friend_key,
                         display_name, sender_key, row[6].toString()};
        } else {
            ToxFile file;
            file.fileKind = TOX_FILE_KIND_DATA;
            file.resumeFileId = row[7].toString().toUtf8();
            file.filePath = row[8].toString();
            file.fileName = row[9].toString();
            file.filesize = row[10].toLongLong();
            file.direction = static_cast<ToxFile::FileDirection>(row[11].toLongLong());
            file.status = static_cast<ToxFile::FileStatus>(row[12].toInt());
            messages +=
                {id, messageState, timestamp, friend_key, display_name, sender_key, file};
        }
    };

    db->execNow({queryText, rowCallback});

    return messages;
}

QList<History::HistMessage> History::getUndeliveredMessagesForFriend(const ToxPk& friendPk)
{
    if (historyAccessBlocked()) {
        return {};
    }

    auto queryText =
        QString("SELECT history.id, faux_offline_pending.id, timestamp, chat.public_key, "
                "aliases.display_name, sender.public_key, message, broken_messages.id "
                "FROM history "
                "JOIN faux_offline_pending ON history.id = faux_offline_pending.id "
                "JOIN peers chat on history.chat_id = chat.id "
                "JOIN aliases on sender_alias = aliases.id "
                "JOIN peers sender on aliases.owner = sender.id "
                "LEFT JOIN broken_messages ON history.id = broken_messages.id "
                "WHERE chat.public_key='%1';")
            .arg(friendPk.toString());

    QList<History::HistMessage> ret;
    auto rowCallback = [&ret](const QVector<QVariant>& row) {
        // dispName and message could have null bytes, QString::fromUtf8
        // truncates on null bytes so we strip them
        auto id = RowId{row[0].toLongLong()};
        auto isPending = !row[1].isNull();
        auto timestamp = QDateTime::fromMSecsSinceEpoch(row[2].toLongLong());
        auto friend_key = row[3].toString();
        auto display_name = QString::fromUtf8(row[4].toByteArray().replace('\0', ""));
        auto sender_key = row[5].toString();
        auto isBroken = !row[7].isNull();

        MessageState messageState = getMessageState(isPending, isBroken);

        ret += {id, messageState, timestamp, friend_key,
                display_name, sender_key, row[6].toString()};
    };

    db->execNow({queryText, rowCallback});

    return ret;
}

/**
 * @brief Search phrase in chat messages
 * @param friendPk Friend public key
 * @param from a date message where need to start a search
 * @param phrase what need to find
 * @param parameter for search
 * @return date of the message where the phrase was found
 */
QDateTime History::getDateWhereFindPhrase(const ToxPk& friendPk, const QDateTime& from,
                                          QString phrase, const ParameterSearch& parameter)
{
    if (historyAccessBlocked()) {
        return QDateTime();
    }

    QDateTime result;
    auto rowCallback = [&result](const QVector<QVariant>& row) {
        result = QDateTime::fromMSecsSinceEpoch(row[0].toLongLong());
    };

    phrase.replace("'", "''");

    QString message;

    switch (parameter.filter) {
    case FilterSearch::Register:
        message = QStringLiteral("message LIKE '%%1%'").arg(phrase);
        break;
    case FilterSearch::WordsOnly:
        message = QStringLiteral("message REGEXP '%1'")
                      .arg(SearchExtraFunctions::generateFilterWordsOnly(phrase).toLower());
        break;
    case FilterSearch::RegisterAndWordsOnly:
        message = QStringLiteral("REGEXPSENSITIVE(message, '%1')")
                      .arg(SearchExtraFunctions::generateFilterWordsOnly(phrase));
        break;
    case FilterSearch::Regular:
        message = QStringLiteral("message REGEXP '%1'").arg(phrase);
        break;
    case FilterSearch::RegisterAndRegular:
        message = QStringLiteral("REGEXPSENSITIVE(message '%1')").arg(phrase);
        break;
    default:
        message = QStringLiteral("LOWER(message) LIKE '%%1%'").arg(phrase.toLower());
        break;
    }

    QDateTime time = from;

    if (!time.isValid()) {
        time = QDateTime::currentDateTime();
    }

    if (parameter.period == PeriodSearch::AfterDate || parameter.period == PeriodSearch::BeforeDate) {
        time = parameter.time;
    }

    QString period;
    switch (parameter.period) {
    case PeriodSearch::WithTheFirst:
        period = QStringLiteral("ORDER BY timestamp ASC LIMIT 1;");
        break;
    case PeriodSearch::AfterDate:
        period = QStringLiteral("AND timestamp > '%1' ORDER BY timestamp ASC LIMIT 1;")
                     .arg(time.toMSecsSinceEpoch());
        break;
    case PeriodSearch::BeforeDate:
        period = QStringLiteral("AND timestamp < '%1' ORDER BY timestamp DESC LIMIT 1;")
                     .arg(time.toMSecsSinceEpoch());
        break;
    default:
        period = QStringLiteral("AND timestamp < '%1' ORDER BY timestamp DESC LIMIT 1;")
                     .arg(time.toMSecsSinceEpoch());
        break;
    }

    QString queryText =
        QStringLiteral("SELECT timestamp "
                       "FROM history "
                       "LEFT JOIN faux_offline_pending ON history.id = faux_offline_pending.id "
                       "JOIN peers chat ON chat_id = chat.id "
                       "WHERE chat.public_key='%1' "
                       "AND %2 "
                       "%3")
            .arg(friendPk.toString())
            .arg(message)
            .arg(period);

    db->execNow({queryText, rowCallback});

    return result;
}

/**
 * @brief Gets date boundaries in conversation with friendPk. History doesn't model conversation indexes,
 * but we can count messages between us and friendPk to effectively give us an index. This function
 * returns how many messages have happened between us <-> friendPk each time the date changes
 * @param[in] friendPk ToxPk of conversation to retrieve
 * @param[in] from Start date to look from
 * @param[in] maxNum Maximum number of date boundaries to retrieve
 * @note This API may seem a little strange, why not use QDate from and QDate to? The intent is to
 * have an API that can be used to get the first item after a date (for search) and to get a list
 * of date changes (for loadHistory). We could write two separate queries but the query is fairly
 * intricate compared to our other ones so reducing duplication of it is preferable.
 */
QList<History::DateIdx> History::getNumMessagesForFriendBeforeDateBoundaries(const ToxPk& friendPk,
                                                                             const QDate& from,
                                                                             size_t maxNum)
{
    if (historyAccessBlocked()) {
        return {};
    }

    auto friendPkString = friendPk.toString();

    // No guarantee that this is the most efficient way to do this...
    // We want to count messages that happened for a friend before a
    // certain date. We do this by re-joining our table a second time
    // but this time with the only filter being that our id is less than
    // the ID of the corresponding row in the table that is grouped by day
    auto countMessagesForFriend =
        QString("SELECT COUNT(*) - 1 " // Count - 1 corresponds to 0 indexed message id for friend
                "FROM history countHistory "            // Import unfiltered table as countHistory
                "JOIN peers chat ON chat_id = chat.id " // link chat_id to chat.id
                "WHERE chat.public_key = '%1'"          // filter this conversation
                "AND countHistory.id <= history.id") // and filter that our unfiltered table history id only has elements up to history.id
            .arg(friendPkString);

    auto limitString = (maxNum) ? QString("LIMIT %1").arg(maxNum) : QString("");

    auto queryString = QString("SELECT (%1), (timestamp / 1000 / 60 / 60 / 24) AS day "
                               "FROM history "
                               "JOIN peers chat ON chat_id = chat.id "
                               "WHERE chat.public_key = '%2' "
                               "AND timestamp >= %3 "
                               "GROUP by day "
                               "%4;")
                           .arg(countMessagesForFriend)
                           .arg(friendPkString)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                           .arg(QDateTime(from.startOfDay()).toMSecsSinceEpoch())
#else
                           .arg(QDateTime(from).toMSecsSinceEpoch())
#endif
                           .arg(limitString);

    QList<DateIdx> dateIdxs;
    auto rowCallback = [&dateIdxs](const QVector<QVariant>& row) {
        DateIdx dateIdx;
        dateIdx.numMessagesIn = row[0].toLongLong();
        dateIdx.date =
            QDateTime::fromMSecsSinceEpoch(row[1].toLongLong() * 24 * 60 * 60 * 1000).date();
        dateIdxs.append(dateIdx);
    };

    db->execNow({queryString, rowCallback});

    return dateIdxs;
}

/**
 * @brief Marks a message as delivered.
 * Removing message from the faux-offline pending messages list.
 *
 * @param id Message ID.
 */
void History::markAsDelivered(RowId messageId)
{
    if (historyAccessBlocked()) {
        return;
    }

    db->execLater(QString("DELETE FROM faux_offline_pending WHERE id=%1;").arg(messageId.get()));
}

/**
* @brief Determines if history access should be blocked
* @return True if history should not be accessed
*/
bool History::historyAccessBlocked()
{
    if (!Settings::getInstance().getEnableLogging()) {
        assert(false);
        qCritical() << "Blocked history access while history is disabled";
        return true;
    }

    if (!isValid()) {
        return true;
    }

    return false;

}
