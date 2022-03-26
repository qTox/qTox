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

#include "to10.h"

#include "src/core/toxpk.h"

#include <QDebug>

bool to10::dbSchema9to10(RawDatabase& db)
{
    // splitting peers table into separate chat and authors table.
    // peers had a dual-meaning of each friend having their own chat, but also each peer
    // being authors of messages. This wasn't true for self which was in the peers table
    // but had no related chat. For upcomming group history, the confusion is amplified
    // since each group is a chat so is in peers, but authors no messages, and each group
    // member is an author so is in peers, but has no chat.
    // Splitting peers makes the relation much clearer and the tables have a single meaning.
    QVector<RawDatabase::Query> upgradeQueries;
    peersToAuthors::peersToAuthors(db, upgradeQueries);
    peersToChats::peersToChats(db, upgradeQueries);
    dropPeers(upgradeQueries);
    upgradeQueries += RawDatabase::Query(QStringLiteral("PRAGMA user_version = 10;"));
    return db.execNow(upgradeQueries);
}

bool to10::peersToAuthors::peersToAuthors(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries)
{
    createNewTables(upgradeQueries);
    if (!populateAuthors(db, upgradeQueries)) {
        return false;
    }
    if (!updateAliasesFk(db, upgradeQueries)) {
        return false;
    }
    updateTextMessagesFk(upgradeQueries);
    updateFileTransfersFk(upgradeQueries);
    replaceOldTables(upgradeQueries);
    return true;
}

void to10::peersToAuthors::createNewTables(QVector<RawDatabase::Query>& upgradeQueries)
{
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE authors (id INTEGER PRIMARY KEY, "
        "public_key BLOB NOT NULL UNIQUE);"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE aliases_new ("
        "id INTEGER PRIMARY KEY, "
        "owner INTEGER, "
        "display_name BLOB NOT NULL, "
        "UNIQUE(owner, display_name), "
        "FOREIGN KEY (owner) REFERENCES authors(id));"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE text_messages_new "
        "(id INTEGER PRIMARY KEY, "
        "message_type CHAR(1) NOT NULL CHECK (message_type = 'T'), "
        "sender_alias INTEGER NOT NULL, "
        "message BLOB NOT NULL, "
        "FOREIGN KEY (id, message_type) REFERENCES history(id, message_type), "
        "FOREIGN KEY (sender_alias) REFERENCES aliases_new(id));"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE file_transfers_new "
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
        "FOREIGN KEY (sender_alias) REFERENCES aliases_new(id));"));
}

bool to10::peersToAuthors::populateAuthors(RawDatabase &db, QVector<RawDatabase::Query> &upgradeQueries)
{
    // don't copy over directly, so that we can convert from string to blob
    return db.execNow(RawDatabase::Query(QStringLiteral(
        "SELECT DISTINCT peers.public_key FROM peers JOIN aliases ON peers.id=aliases.owner"),
        [&](const QVector<QVariant>& row) {
            upgradeQueries += RawDatabase::Query(QStringLiteral(
                "INSERT INTO authors (public_key) VALUES (?)"),
                {ToxPk{row[0].toString()}.getByteArray()});
        }
    ));
}

bool to10::peersToAuthors::updateAliasesFk(RawDatabase &db, QVector<RawDatabase::Query>& upgradeQueries)
{
    return db.execNow(RawDatabase::Query(QStringLiteral(
        "SELECT id, public_key FROM peers"),
        [&](const QVector<QVariant>& row) {
            const auto oldPeerId = row[0].toLongLong();
            const auto pk = ToxPk{row[1].toString()};
            upgradeQueries += RawDatabase::Query(QStringLiteral(
                "INSERT INTO aliases_new (id, owner, display_name) "
                "SELECT id, "
                "(SELECT id FROM authors WHERE public_key = ?), "
                "display_name "
                "FROM aliases WHERE "
                "owner = '%1';").arg(oldPeerId),
                {pk.getByteArray()});
        }));
}

void to10::peersToAuthors::replaceOldTables(QVector<RawDatabase::Query>& upgradeQueries) {
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE text_messages;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE text_messages_new RENAME TO text_messages;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE file_transfers;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE file_transfers_new RENAME TO file_transfers;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE aliases;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE aliases_new RENAME TO aliases;"));
}

bool to10::peersToChats::peersToChats(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries)
{
    createNewTables(upgradeQueries);
    if (!populateChats(db, upgradeQueries)) {
        return false;
    }
    if (!updateHistoryFk(db, upgradeQueries)) {
        return false;
    }
    updateTextMessagesFk(upgradeQueries);
    updateFileTransfersFk(upgradeQueries);
    updateSystemMessagesFk(upgradeQueries);
    updateFauxOfflinePendingFk(upgradeQueries);
    updateBrokenMessagesFk(upgradeQueries);
    replaceOldTables(upgradeQueries);
    return true;
}

void to10::peersToChats::createNewTables(QVector<RawDatabase::Query>& upgradeQueries)
{
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE chats (id INTEGER PRIMARY KEY, "
        "uuid BLOB NOT NULL UNIQUE);"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE history_new "
        "(id INTEGER PRIMARY KEY, "
        "message_type CHAR(1) NOT NULL DEFAULT 'T' CHECK (message_type in ('T','F','S')), "
        "timestamp INTEGER NOT NULL, "
        "chat_id INTEGER NOT NULL, "
        "UNIQUE (id, message_type), "
        "FOREIGN KEY (chat_id) REFERENCES chats(id));"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE text_messages_new "
        "(id INTEGER PRIMARY KEY, "
        "message_type CHAR(1) NOT NULL CHECK (message_type = 'T'), "
        "sender_alias INTEGER NOT NULL, "
        "message BLOB NOT NULL, "
        "FOREIGN KEY (id, message_type) REFERENCES history_new(id, message_type), "
        "FOREIGN KEY (sender_alias) REFERENCES aliases(id));"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE file_transfers_new "
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
        "FOREIGN KEY (id, message_type) REFERENCES history_new(id, message_type), "
        "FOREIGN KEY (sender_alias) REFERENCES aliases(id));"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE system_messages_new "
        "(id INTEGER PRIMARY KEY, "
        "message_type CHAR(1) NOT NULL CHECK (message_type = 'S'), "
        "system_message_type INTEGER NOT NULL, "
        "arg1 BLOB, "
        "arg2 BLOB, "
        "arg3 BLOB, "
        "arg4 BLOB, "
        "FOREIGN KEY (id, message_type) REFERENCES history_new(id, message_type));"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE faux_offline_pending_new (id INTEGER PRIMARY KEY, "
        "required_extensions INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (id) REFERENCES history_new(id));"));
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE broken_messages_new (id INTEGER PRIMARY KEY, "
        "reason INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (id) REFERENCES history_new(id));"));
}

bool to10::peersToChats::populateChats(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries)
{
    // don't copy over directly, so that we can convert from string to blob
    return db.execNow(RawDatabase::Query(QStringLiteral(
        "SELECT DISTINCT peers.public_key FROM peers JOIN history ON peers.id=history.chat_id"),
        [&](const QVector<QVariant>& row) {
            upgradeQueries += RawDatabase::Query(QStringLiteral(
                "INSERT INTO chats (uuid) VALUES (?)"),
                {ToxPk{row[0].toString()}.getByteArray()});
        }
    ));
}

bool to10::peersToChats::updateHistoryFk(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries)
{
    return db.execNow(RawDatabase::Query(QStringLiteral(
        "SELECT DISTINCT peers.id, peers.public_key FROM peers JOIN history ON peers.id=history.chat_id"),
        [&](const QVector<QVariant>& row) {
            const auto oldPeerId = row[0].toLongLong();
            const auto pk = ToxPk{row[1].toString()};
            upgradeQueries += RawDatabase::Query(QStringLiteral(
                "INSERT INTO history_new (id, message_type, timestamp, chat_id) "
                "SELECT id, "
                "message_type, "
                "timestamp, "
                "(SELECT id FROM chats WHERE uuid = ?) "
                "FROM history WHERE chat_id = '%1'").arg(oldPeerId),
                {pk.getByteArray()});
        }));
}

void to10::peersToChats::replaceOldTables(QVector<RawDatabase::Query>& upgradeQueries) {
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE text_messages;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE text_messages_new RENAME TO text_messages;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE file_transfers;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE file_transfers_new RENAME TO file_transfers;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE system_messages;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE system_messages_new RENAME TO system_messages;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE faux_offline_pending;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE faux_offline_pending_new RENAME TO faux_offline_pending;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE broken_messages;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE broken_messages_new RENAME TO broken_messages;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE history;"));
    upgradeQueries += RawDatabase::Query(QStringLiteral("ALTER TABLE history_new RENAME TO history;"));
}

void to10::updateTextMessagesFk(QVector<RawDatabase::Query>& upgradeQueries)
{
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO text_messages_new SELECT * FROM text_messages"));
}

void to10::updateFileTransfersFk(QVector<RawDatabase::Query>& upgradeQueries)
{
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO file_transfers_new SELECT * FROM file_transfers"));
}

void to10::peersToChats::updateSystemMessagesFk(QVector<RawDatabase::Query>& upgradeQueries)
{
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO system_messages_new SELECT * FROM system_messages"));
}

void to10::peersToChats::updateFauxOfflinePendingFk(QVector<RawDatabase::Query>& upgradeQueries)
{
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO faux_offline_pending_new SELECT * FROM faux_offline_pending"));
}

void to10::peersToChats::updateBrokenMessagesFk(QVector<RawDatabase::Query>& upgradeQueries)
{
    upgradeQueries += RawDatabase::Query(QStringLiteral(
        "INSERT INTO broken_messages_new SELECT * FROM broken_messages"));
}

void to10::dropPeers(QVector<RawDatabase::Query>& upgradeQueries)
{
    upgradeQueries += RawDatabase::Query(QStringLiteral("DROP TABLE peers;"));
}
