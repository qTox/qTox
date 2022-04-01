/*
    Copyright © 2022 by The qTox Project Contributors

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

#include "dbupgrader.h"
#include "src/core/chatid.h"
#include "src/core/toxpk.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/persistence/db/upgrades/dbto11.h"
#include "src/widget/tool/imessageboxmanager.h"

#include <QDebug>
#include <QString>
#include <QTranslator>

namespace {
constexpr int SCHEMA_VERSION = 11;

std::vector<DbUpgrader::BadEntry> getInvalidPeers(RawDatabase& db)
{
    std::vector<DbUpgrader::BadEntry> badPeerIds;
    db.execNow(
        RawDatabase::Query("SELECT id, public_key FROM peers WHERE LENGTH(public_key) != 64",
                           [&](const QVector<QVariant>& row) {
                               badPeerIds.emplace_back(DbUpgrader::BadEntry{row[0].toInt(), row[1].toString()});
                           }));
    return badPeerIds;
}

RowId getValidPeerRow(RawDatabase& db, const ChatId& chatId)
{
    bool validPeerExists{false};
    RowId validPeerRow;
    db.execNow(RawDatabase::Query(QStringLiteral("SELECT id FROM peers WHERE CAST(public_key AS BLOB)=?;"),
        // Note: The conversion to string then back to binary is intentional to
        // ensure we're using the binary presentation of the upper case ASCII
        // representation of the binary key, since we want to find the uppercase
        // entry or insert it ourselves. This is needed for the dbTo11 upgrade.
                                    {chatId.toString().toUtf8()},
                                  [&](const QVector<QVariant>& row) {
                                      validPeerRow = RowId{row[0].toLongLong()};
                                      validPeerExists = true;
                                  }));
    if (validPeerExists) {
        return validPeerRow;
    }

    db.execNow(RawDatabase::Query(("SELECT id FROM peers ORDER BY id DESC LIMIT 1;"),
                                  [&](const QVector<QVariant>& row) {
                                      int64_t maxPeerId = row[0].toInt();
                                      validPeerRow = RowId{maxPeerId + 1};
                                  }));
    db.execNow(
        RawDatabase::Query(QStringLiteral("INSERT INTO peers (id, public_key) VALUES (%1, '%2');")
                               .arg(validPeerRow.get())
                               .arg(chatId.toString())));
    return validPeerRow;
}

struct DuplicateAlias
{
    DuplicateAlias(RowId goodAliasRow_, std::vector<RowId> badAliasRows_)
        : goodAliasRow{goodAliasRow_}
        , badAliasRows{badAliasRows_}
    {}
    DuplicateAlias(){};
    RowId goodAliasRow{-1};
    std::vector<RowId> badAliasRows;
};

DuplicateAlias getDuplicateAliasRows(RawDatabase& db, RowId goodPeerRow, RowId badPeerRow)
{
    std::vector<RowId> badAliasRows;
    RowId goodAliasRow;
    bool hasGoodEntry{false};
    db.execNow(RawDatabase::Query(
        QStringLiteral("SELECT good.id, bad.id FROM aliases good INNER JOIN aliases bad ON "
                       "good.display_name=bad.display_name WHERE good.owner=%1 AND bad.owner=%2;")
            .arg(goodPeerRow.get())
            .arg(badPeerRow.get()),
        [&](const QVector<QVariant>& row) {
            hasGoodEntry = true;
            goodAliasRow = RowId{row[0].toInt()};
            badAliasRows.emplace_back(RowId{row[1].toLongLong()});
        }));

    if (hasGoodEntry) {
        return {goodAliasRow, badAliasRows};
    } else {
        return {};
    }
}

void mergeAndDeleteAlias(QVector<RawDatabase::Query>& upgradeQueries, RowId goodAlias,
                         std::vector<RowId> badAliases)
{
    for (const auto badAliasId : badAliases) {
        upgradeQueries += RawDatabase::Query(
            QStringLiteral("UPDATE text_messages SET sender_alias = %1 WHERE sender_alias = %2;")
                .arg(goodAlias.get())
                .arg(badAliasId.get()));
        upgradeQueries += RawDatabase::Query(
            QStringLiteral("UPDATE file_transfers SET sender_alias = %1 WHERE sender_alias = %2;")
                .arg(goodAlias.get())
                .arg(badAliasId.get()));
        upgradeQueries += RawDatabase::Query(
            QStringLiteral("DELETE FROM aliases WHERE id = %1;").arg(badAliasId.get()));
    }
}

void mergeAndDeletePeer(QVector<RawDatabase::Query>& upgradeQueries, RowId goodPeerId, RowId badPeerId)
{
    upgradeQueries +=
        RawDatabase::Query(QStringLiteral("UPDATE aliases SET owner = %1 WHERE owner = %2")
                               .arg(goodPeerId.get())
                               .arg(badPeerId.get()));
    upgradeQueries +=
        RawDatabase::Query(QStringLiteral("UPDATE history SET chat_id = %1 WHERE chat_id = %2;")
                               .arg(goodPeerId.get())
                               .arg(badPeerId.get()));
    upgradeQueries +=
        RawDatabase::Query(QStringLiteral("DELETE FROM peers WHERE id = %1").arg(badPeerId.get()));
}

void addForeignKeyToAlias(QVector<RawDatabase::Query>& queries)
{
    queries += RawDatabase::Query(
        QStringLiteral("CREATE TABLE aliases_new (id INTEGER PRIMARY KEY, owner INTEGER, "
                       "display_name BLOB NOT NULL, UNIQUE(owner, display_name), "
                       "FOREIGN KEY (owner) REFERENCES peers(id));"));
    queries +=
        RawDatabase::Query(QStringLiteral("INSERT INTO aliases_new (id, owner, display_name) "
                                          "SELECT id, owner, display_name "
                                          "FROM aliases;"));
    queries += RawDatabase::Query(QStringLiteral("DROP TABLE aliases;"));
    queries += RawDatabase::Query(QStringLiteral("ALTER TABLE aliases_new RENAME TO aliases;"));
}

void addForeignKeyToHistory(QVector<RawDatabase::Query>& queries)
{
    queries +=
        RawDatabase::Query(QStringLiteral("CREATE TABLE history_new "
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
    queries += RawDatabase::Query(
        QStringLiteral("CREATE TABLE new_faux_offline_pending (id INTEGER PRIMARY KEY, "
                       "FOREIGN KEY (id) REFERENCES history(id));"));
    queries += RawDatabase::Query(QStringLiteral("INSERT INTO new_faux_offline_pending (id) "
                                                 "SELECT id "
                                                 "FROM faux_offline_pending;"));
    queries += RawDatabase::Query(QStringLiteral("DROP TABLE faux_offline_pending;"));
    queries += RawDatabase::Query(
        QStringLiteral("ALTER TABLE new_faux_offline_pending RENAME TO faux_offline_pending;"));
}

void addForeignKeyToBrokenMessages(QVector<RawDatabase::Query>& queries)
{
    queries += RawDatabase::Query(
        QStringLiteral("CREATE TABLE new_broken_messages (id INTEGER PRIMARY KEY, "
                       "FOREIGN KEY (id) REFERENCES history(id));"));
    queries += RawDatabase::Query(QStringLiteral("INSERT INTO new_broken_messages (id) "
                                                 "SELECT id "
                                                 "FROM broken_messages;"));
    queries += RawDatabase::Query(QStringLiteral("DROP TABLE broken_messages;"));
    queries += RawDatabase::Query(
        QStringLiteral("ALTER TABLE new_broken_messages RENAME TO broken_messages;"));
}
} // namespace

/**
 * @brief Upgrade the db schema
 * @note On future alterations of the database all you have to do is bump the SCHEMA_VERSION
 * variable and add another case to the switch statement below. Make sure to fall through on each case.
 */
bool DbUpgrader::dbSchemaUpgrade(std::shared_ptr<RawDatabase>& db, IMessageBoxManager& messageBoxManager)
{
    // If we're a new dB we can just make a new one and call it a day
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
        return true;
    }

    // Otherwise we have to do upgrades from our current version to the latest version

    int64_t databaseSchemaVersion;

    if (!db->execNow(RawDatabase::Query("PRAGMA user_version", [&](const QVector<QVariant>& row) {
            databaseSchemaVersion = row[0].toLongLong();
        }))) {
        qCritical() << "History failed to read user_version";
        return false;
    }

    if (databaseSchemaVersion > SCHEMA_VERSION) {
        messageBoxManager.showError(QObject::tr("Failed to load chat history"),
            QObject::tr("Database version (%1) is newer than we currently support (%2). Please upgrade qTox.")
            .arg(databaseSchemaVersion).arg(SCHEMA_VERSION));
        qWarning().nospace() << "Database version (" << databaseSchemaVersion
                             << ") is newer than we currently support (" << SCHEMA_VERSION
                             << "). Please upgrade qTox";
        // We don't know what future versions have done, we have to disable db access until we re-upgrade
        return false;
    } else if (databaseSchemaVersion == SCHEMA_VERSION) {
        // No work to do
        return true;
    }

    using DbSchemaUpgradeFn = bool (*)(RawDatabase&);
    std::vector<DbSchemaUpgradeFn> upgradeFns = {dbSchema0to1, dbSchema1to2, dbSchema2to3,
                                                 dbSchema3to4, dbSchema4to5, dbSchema5to6,
                                                 dbSchema6to7, dbSchema7to8, dbSchema8to9,
                                                 dbSchema9to10, DbTo11::dbSchema10to11};

    assert(databaseSchemaVersion < static_cast<int>(upgradeFns.size()));
    assert(upgradeFns.size() == SCHEMA_VERSION);

    for (int64_t i = databaseSchemaVersion; i < static_cast<int>(upgradeFns.size()); ++i) {
        auto const newDbVersion = i + 1;
        if (!upgradeFns[i](*db)) {
            qCritical() << "Failed to upgrade db to schema version " << newDbVersion << " aborting";
            return false;
        }
        qDebug() << "Database upgraded incrementally to schema version " << newDbVersion;
    }

    qInfo() << "Database upgrade finished (databaseSchemaVersion" << databaseSchemaVersion << "->"
            << SCHEMA_VERSION << ")";
    return true;
}

bool DbUpgrader::createCurrentSchema(RawDatabase& db)
{
    QVector<RawDatabase::Query> queries;
    queries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE authors (id INTEGER PRIMARY KEY, "
        "public_key BLOB NOT NULL UNIQUE);"
        "CREATE TABLE chats (id INTEGER PRIMARY KEY, "
        "uuid BLOB NOT NULL UNIQUE);"
        "CREATE TABLE aliases (id INTEGER PRIMARY KEY, "
        "owner INTEGER, "
        "display_name BLOB NOT NULL, "
        "UNIQUE(owner, display_name), "
        "FOREIGN KEY (owner) REFERENCES authors(id));"
        "CREATE TABLE history "
        "(id INTEGER PRIMARY KEY, "
        "message_type CHAR(1) NOT NULL DEFAULT 'T' CHECK (message_type in ('T','F','S')), "
        "timestamp INTEGER NOT NULL, "
        "chat_id INTEGER NOT NULL, "
        // Message subtypes want to reference the following as a foreign key. Foreign keys must be
        // guaranteed to be unique. Since an ID is already unique, id + message type is also unique
        "UNIQUE (id, message_type), "
        "FOREIGN KEY (chat_id) REFERENCES chats(id)); "
        "CREATE TABLE text_messages "
        "(id INTEGER PRIMARY KEY, "
        "message_type CHAR(1) NOT NULL CHECK (message_type = 'T'), "
        "sender_alias INTEGER NOT NULL, "
        // even though technically a message can be null for file transfer, we've opted
        // to just insert an empty string when there's no content, this moderately simplifies
        // implementation as currently our database doesn't have support for optional fields.
        // We would either have to insert "?" or "null" based on if message exists and then
        // ensure that our blob vector always has the right number of fields. Better to just
        // leave this as NOT NULL for now.
        "message BLOB NOT NULL, "
        "FOREIGN KEY (id, message_type) REFERENCES history(id, message_type), "
        "FOREIGN KEY (sender_alias) REFERENCES aliases(id)); "
        "CREATE TABLE file_transfers "
        "(id INTEGER PRIMARY KEY, "
        "message_type CHAR(1) NOT NULL CHECK (message_type = 'F'), "
        "sender_alias INTEGER NOT NULL, "
        "file_restart_id BLOB NOT NULL, "
        "file_name BLOB NOT NULL, "
        "file_path BLOB NOT NULL, "
        "file_hash BLOB NOT NULL, "
        "file_size INTEGER NOT NULL, "
        "direction INTEGER NOT NULL, "
        "file_state INTEGER NOT NULL, "
        "FOREIGN KEY (id, message_type) REFERENCES history(id, message_type), "
        "FOREIGN KEY (sender_alias) REFERENCES aliases(id)); "
        "CREATE TABLE system_messages "
        "(id INTEGER PRIMARY KEY, "
        "message_type CHAR(1) NOT NULL CHECK (message_type = 'S'), "
        "system_message_type INTEGER NOT NULL, "
        "arg1 BLOB, "
        "arg2 BLOB, "
        "arg3 BLOB, "
        "arg4 BLOB, "
        "FOREIGN KEY (id, message_type) REFERENCES history(id, message_type)); "
        "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY, "
        "required_extensions INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (id) REFERENCES history(id));"
        "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY, "
        "reason INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (id) REFERENCES history(id));"));
    // sqlite doesn't support including the index as part of the CREATE TABLE statement, so add a second query
    queries += RawDatabase::Query(
        "CREATE INDEX chat_id_idx on history (chat_id);");
    queries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = %1;").arg(SCHEMA_VERSION));
    return db.execNow(queries);
}

bool DbUpgrader::isNewDb(std::shared_ptr<RawDatabase>& db, bool& success)
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

bool DbUpgrader::dbSchema0to1(RawDatabase& db)
{
    QVector<RawDatabase::Query> queries;
    queries += RawDatabase::Query(QStringLiteral("CREATE TABLE file_transfers "
                                                 "(id INTEGER PRIMARY KEY, "
                                                 "chat_id INTEGER NOT NULL, "
                                                 "file_restart_id BLOB NOT NULL, "
                                                 "file_name BLOB NOT NULL, "
                                                 "file_path BLOB NOT NULL, "
                                                 "file_hash BLOB NOT NULL, "
                                                 "file_size INTEGER NOT NULL, "
                                                 "direction INTEGER NOT NULL, "
                                                 "file_state INTEGER NOT NULL);"));
    queries += RawDatabase::Query(QStringLiteral("ALTER TABLE history ADD file_id INTEGER;"));
    queries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 1;"));
    return db.execNow(queries);
}

bool DbUpgrader::dbSchema1to2(RawDatabase& db)
{
    // Any faux_offline_pending message, in a chat that has newer delivered
    // message is decided to be broken. It must be moved from
    // faux_offline_pending to broken_messages

    // the last non-pending message in each chat
    QString lastDeliveredQuery =
        QString("SELECT chat_id, MAX(history.id) FROM "
                "history JOIN peers chat ON chat_id = chat.id "
                "LEFT JOIN faux_offline_pending ON history.id = faux_offline_pending.id "
                "WHERE faux_offline_pending.id IS NULL "
                "GROUP BY chat_id;");

    QVector<RawDatabase::Query> upgradeQueries;
    upgradeQueries += RawDatabase::Query(QStringLiteral("CREATE TABLE broken_messages "
                                                        "(id INTEGER PRIMARY KEY);"));

    auto rowCallback = [&upgradeQueries](const QVector<QVariant>& row) {
        auto chatId = row[0].toLongLong();
        auto lastDeliveredHistoryId = row[1].toLongLong();

        upgradeQueries += QString("INSERT INTO broken_messages "
                                  "SELECT faux_offline_pending.id FROM "
                                  "history JOIN faux_offline_pending "
                                  "ON faux_offline_pending.id = history.id "
                                  "WHERE history.chat_id=%1 "
                                  "AND history.id < %2;")
                              .arg(chatId)
                              .arg(lastDeliveredHistoryId);
    };
    // note this doesn't modify the db, just generate new queries, so is safe
    // to run outside of our upgrade transaction
    if (!db.execNow({lastDeliveredQuery, rowCallback})) {
        return false;
    }

    upgradeQueries += QString("DELETE FROM faux_offline_pending "
                              "WHERE id in ("
                              "SELECT id FROM broken_messages);");

    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 2;"));

    return db.execNow(upgradeQueries);
}

bool DbUpgrader::dbSchema2to3(RawDatabase& db)
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

    upgradeQueries += QString("DELETE FROM faux_offline_pending "
                              "WHERE id in ("
                              "SELECT id FROM broken_messages);");

    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 3;"));

    return db.execNow(upgradeQueries);
}

bool DbUpgrader::dbSchema3to4(RawDatabase& db)
{
    QVector<RawDatabase::Query> upgradeQueries;
    upgradeQueries += RawDatabase::Query{QString("CREATE INDEX chat_id_idx on history (chat_id);")};

    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 4;"));

    return db.execNow(upgradeQueries);
}

bool DbUpgrader::dbSchema4to5(RawDatabase& db)
{
    // add foreign key contrains to database tables. sqlite doesn't support advanced alter table
    // commands, so instead we need to copy data to new tables with the foreign key contraints:
    // http://www.sqlitetutorial.net/sqlite-alter-table/
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

bool DbUpgrader::dbSchema5to6(RawDatabase& db)
{
    QVector<RawDatabase::Query> upgradeQueries;

    upgradeQueries += RawDatabase::Query{QString("ALTER TABLE faux_offline_pending "
                                                 "ADD COLUMN required_extensions INTEGER NOT NULL "
                                                 "DEFAULT 0;")};

    upgradeQueries += RawDatabase::Query{QString("ALTER TABLE broken_messages "
                                                 "ADD COLUMN reason INTEGER NOT NULL "
                                                 "DEFAULT 0;")};

    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 6;"));
    return db.execNow(upgradeQueries);
}

bool DbUpgrader::dbSchema6to7(RawDatabase& db)
{
    QVector<RawDatabase::Query> upgradeQueries;

    // Cannot add UNIQUE(id, message_type) to history table without creating a new one. Create a new history table
    upgradeQueries += RawDatabase::Query(
        "CREATE TABLE history_new (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL DEFAULT "
        "'T' CHECK (message_type in ('T','F','S')), timestamp INTEGER NOT NULL, chat_id INTEGER "
        "NOT NULL, UNIQUE (id, message_type), FOREIGN KEY (chat_id) REFERENCES peers(id))");

    // Create new text_messages table. We will split messages out of history and insert them into this new table
    upgradeQueries += RawDatabase::Query(
        "CREATE TABLE text_messages (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL CHECK "
        "(message_type = 'T'), sender_alias INTEGER NOT NULL, message BLOB NOT NULL, FOREIGN KEY "
        "(id, message_type) REFERENCES history_new(id, message_type), FOREIGN KEY (sender_alias) "
        "REFERENCES aliases(id))");

    // Cannot add a FOREIGN KEY to the file_transfers table without creating a new one. Create a new file_transfers table
    upgradeQueries += RawDatabase::Query(
        "CREATE TABLE file_transfers_new (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL "
        "CHECK (message_type = 'F'), sender_alias INTEGER NOT NULL, file_restart_id BLOB NOT NULL, "
        "file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size "
        "INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL, FOREIGN KEY "
        "(id, message_type) REFERENCES history_new(id, message_type), FOREIGN KEY (sender_alias) "
        "REFERENCES aliases(id))");

    upgradeQueries +=
        RawDatabase::Query("INSERT INTO history_new SELECT id, 'T' AS message_type, timestamp, "
                           "chat_id FROM history WHERE history.file_id IS NULL");

    upgradeQueries +=
        RawDatabase::Query("INSERT INTO text_messages SELECT id, 'T' AS message_type, "
                           "sender_alias, message FROM history WHERE history.file_id IS NULL");

    upgradeQueries +=
        RawDatabase::Query("INSERT INTO history_new SELECT id, 'F' AS message_type, timestamp, "
                           "chat_id FROM history WHERE history.file_id IS NOT NULL");

    upgradeQueries += RawDatabase::Query(
        "INSERT INTO file_transfers_new (id, message_type, sender_alias, file_restart_id, "
        "file_name, file_path, file_hash, file_size, direction, file_state) SELECT history.id, 'F' "
        "as message_type, history.sender_alias, file_transfers.file_restart_id, "
        "file_transfers.file_name, file_transfers.file_path, file_transfers.file_hash, "
        "file_transfers.file_size, file_transfers.direction, file_transfers.file_state FROM "
        "history INNER JOIN file_transfers on history.file_id = file_transfers.id WHERE "
        "history.file_id IS NOT NULL");

    upgradeQueries += RawDatabase::Query(
        "CREATE TABLE system_messages (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL CHECK "
        "(message_type = 'S'), system_message_type INTEGER NOT NULL, arg1 BLOB, arg2 BLOB, arg3 "
        "BLOB, arg4 BLOB, "
        "FOREIGN KEY (id, message_type) REFERENCES history_new(id, message_type))");

    // faux_offline_pending needs to be re-created to reference the new history table
    upgradeQueries += RawDatabase::Query(
        "CREATE TABLE faux_offline_pending_new (id INTEGER PRIMARY KEY, required_extensions "
        "INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (id) REFERENCES history_new(id))");
    upgradeQueries += RawDatabase::Query("INSERT INTO faux_offline_pending_new SELECT id, "
                                         "required_extensions FROM faux_offline_pending");
    upgradeQueries += RawDatabase::Query("DROP TABLE faux_offline_pending");
    upgradeQueries +=
        RawDatabase::Query("ALTER TABLE faux_offline_pending_new RENAME TO faux_offline_pending");

    // broken_messages needs to be re-created to reference the new history table
    upgradeQueries += RawDatabase::Query(
        "CREATE TABLE broken_messages_new (id INTEGER PRIMARY KEY, reason INTEGER NOT NULL DEFAULT "
        "0, FOREIGN KEY (id) REFERENCES history_new(id))");
    upgradeQueries += RawDatabase::Query(
        "INSERT INTO broken_messages_new SELECT id, reason FROM broken_messages");
    upgradeQueries += RawDatabase::Query("DROP TABLE broken_messages");
    upgradeQueries +=
        RawDatabase::Query("ALTER TABLE broken_messages_new RENAME TO broken_messages");

    // Everything referencing old history should now be gone
    upgradeQueries += RawDatabase::Query("DROP TABLE history");
    upgradeQueries += RawDatabase::Query("ALTER TABLE history_new RENAME TO history");

    // Drop file transfers late since history depends on it
    upgradeQueries += RawDatabase::Query("DROP TABLE file_transfers");
    upgradeQueries += RawDatabase::Query("ALTER TABLE file_transfers_new RENAME TO file_transfers");

    upgradeQueries += RawDatabase::Query("CREATE INDEX chat_id_idx on history (chat_id);");

    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 7;"));

    return db.execNow(upgradeQueries);
}

bool DbUpgrader::dbSchema7to8(RawDatabase& db)
{
    // Dummy upgrade. This upgrade does not change the schema, however on
    // version 7 if qtox saw a system message it would assert and crash. This
    // upgrade ensures that old versions of qtox do not try to load the new
    // database

    QVector<RawDatabase::Query> upgradeQueries;
    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 8;"));

    return db.execNow(upgradeQueries);
}

bool DbUpgrader::dbSchema8to9(RawDatabase& db)
{
    // not technically a schema update, but still a database version update based on healing invalid user data
    // we added ourself in the peers table by ToxId isntead of ToxPk. Heal this over-length entry.
    QVector<RawDatabase::Query> upgradeQueries;
    const auto badPeers = getInvalidPeers(db);
    mergeDuplicatePeers(upgradeQueries, db, badPeers);
    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 9;"));
    return db.execNow(upgradeQueries);
}

bool DbUpgrader::dbSchema9to10(RawDatabase& db)
{
    // not technically a schema update, but still a database version update based on healing invalid user data.
    // We inserted some empty resume_file_id's due to #6553. Heal the under-length entries.
    // The resume file ID isn't actually used for loaded files at this time, so we can heal it to an arbitrary
    // value of full length.
    constexpr int resumeFileIdLengthNow = 32;
    QByteArray dummyResumeId(resumeFileIdLengthNow, 0);
    QVector<RawDatabase::Query> upgradeQueries;
    upgradeQueries += RawDatabase::Query(QStringLiteral("UPDATE file_transfers SET file_restart_id = ? WHERE LENGTH(file_restart_id) != 32;"),
        {dummyResumeId});
    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 10;"));
    return db.execNow(upgradeQueries);
}

void DbUpgrader::mergeDuplicatePeers(QVector<RawDatabase::Query>& upgradeQueries, RawDatabase& db,
                         std::vector<BadEntry> badPeers)
{
    for (const auto& badPeer : badPeers) {
        const RowId goodPeerId = getValidPeerRow(db, ToxPk{badPeer.toxId.left(64)});
        const auto aliasDuplicates = getDuplicateAliasRows(db, goodPeerId, badPeer.row);
        mergeAndDeleteAlias(upgradeQueries, aliasDuplicates.goodAliasRow, aliasDuplicates.badAliasRows);
        mergeAndDeletePeer(upgradeQueries, goodPeerId, badPeer.row);
    }
}
