/*
    Copyright © 2019 by The qTox Project Contributors

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

#include "dbutility/dbutility.h"
#include "src/persistence/db/rawdatabase.h"

#include <QTest>

#include <array>
#include <algorithm>
#include <vector>

const std::array<QString,11> DbUtility::testFileList =
    {"testCreation.db", "testIsNewDbTrue.db", "testIsNewDbFalse.db",
     "test0to1.db",     "test1to2.db",        "test2to3.db",
     "test3to4.db",     "test4to5.db",        "test5to6.db",
     "test6to7.db",     "test9to10.db"};

// db schemas can be select with "SELECT name, sql FROM sqlite_master;" on the database.
const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema0 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL)"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"}
};

// added file transfer history
const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema1 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
    {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER)"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"}
};

// move stuck faux offline messages do a table of "broken" messages
const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema2 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
    {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER)"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"},
    {"broken_messages", "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY)"}
};

// move stuck 0-length action messages to the existing "broken_messages" table. Not a real schema upgrade.
const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema3 = DbUtility::schema2;

// create index in history table on chat_id to improve query speed. Not a real schema upgrade.
const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema4 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
    {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER)"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"},
    {"broken_messages", "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY)"},
    {"chat_id_idx", "CREATE INDEX chat_id_idx on history (chat_id)"}
};

// added foreign keys
const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema5 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name), FOREIGN KEY (owner) REFERENCES peers(id))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY, FOREIGN KEY (id) REFERENCES history(id))"},
    {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER, FOREIGN KEY (file_id) REFERENCES file_transfers(id), FOREIGN KEY (chat_id) REFERENCES peers(id), FOREIGN KEY (sender_alias) REFERENCES aliases(id))"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"},
    {"broken_messages", "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY, FOREIGN KEY (id) REFERENCES history(id))"},
    {"chat_id_idx", "CREATE INDEX chat_id_idx on history (chat_id)"}
};

// added toxext extensions
const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema6 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name), FOREIGN KEY (owner) REFERENCES peers(id))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY, required_extensions INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (id) REFERENCES history(id))"},
    {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER, FOREIGN KEY (file_id) REFERENCES file_transfers(id), FOREIGN KEY (chat_id) REFERENCES peers(id), FOREIGN KEY (sender_alias) REFERENCES aliases(id))"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"},
    {"broken_messages", "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY, reason INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (id) REFERENCES history(id))"},
    {"chat_id_idx", "CREATE INDEX chat_id_idx on history (chat_id)"}
};

const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema7 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB "
                "NOT NULL, UNIQUE(owner, display_name), FOREIGN KEY (owner) REFERENCES peers(id))"},
    {"faux_offline_pending",
     "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY, required_extensions INTEGER NOT "
     "NULL DEFAULT 0, FOREIGN KEY (id) REFERENCES history(id))"},
    {"file_transfers",
     "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL CHECK "
     "(message_type = 'F'), sender_alias INTEGER NOT NULL, file_restart_id BLOB NOT NULL, "
     "file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER "
     "NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL, FOREIGN KEY (id, "
     "message_type) REFERENCES history(id, message_type), FOREIGN KEY (sender_alias) REFERENCES "
     "aliases(id))"},
    {"history",
     "CREATE TABLE history (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL DEFAULT 'T' "
     "CHECK (message_type in ('T','F','S')), timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, "
     "UNIQUE (id, message_type), FOREIGN KEY (chat_id) REFERENCES peers(id))"},
    {"text_messages", "CREATE TABLE text_messages (id INTEGER PRIMARY KEY, message_type CHAR(1) "
                      "NOT NULL CHECK (message_type = 'T'), sender_alias INTEGER NOT NULL, message "
                      "BLOB NOT NULL, FOREIGN KEY (id, message_type) REFERENCES history(id, "
                      "message_type), FOREIGN KEY (sender_alias) REFERENCES aliases(id))"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"},
    {"broken_messages", "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY, reason INTEGER NOT "
                        "NULL DEFAULT 0, FOREIGN KEY (id) REFERENCES history(id))"},
    {"system_messages",
     "CREATE TABLE system_messages (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL CHECK "
     "(message_type = 'S'), system_message_type INTEGER NOT NULL, arg1 BLOB, arg2 BLOB, arg3 BLOB, arg4 BLOB, "
     "FOREIGN KEY (id, message_type) REFERENCES history(id, message_type))"},
    {"chat_id_idx", "CREATE INDEX chat_id_idx on history (chat_id)"}};

const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema9 = DbUtility::schema7;
const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema10 = DbUtility::schema9;

const std::vector<DbUtility::SqliteMasterEntry> DbUtility::schema11{
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB "
                "NOT NULL, UNIQUE(owner, display_name), FOREIGN KEY (owner) REFERENCES authors(id))"},
    {"faux_offline_pending",
     "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY, required_extensions INTEGER NOT "
     "NULL DEFAULT 0, FOREIGN KEY (id) REFERENCES history(id))"},
    {"file_transfers",
     "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL CHECK "
     "(message_type = 'F'), sender_alias INTEGER NOT NULL, file_restart_id BLOB NOT NULL, "
     "file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER "
     "NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL, FOREIGN KEY (id, "
     "message_type) REFERENCES history(id, message_type), FOREIGN KEY (sender_alias) REFERENCES "
     "aliases(id))"},
    {"history",
     "CREATE TABLE history (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL DEFAULT 'T' "
     "CHECK (message_type in ('T','F','S')), timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, "
     "UNIQUE (id, message_type), FOREIGN KEY (chat_id) REFERENCES chats(id))"},
    {"text_messages", "CREATE TABLE text_messages (id INTEGER PRIMARY KEY, message_type CHAR(1) "
                      "NOT NULL CHECK (message_type = 'T'), sender_alias INTEGER NOT NULL, message "
                      "BLOB NOT NULL, FOREIGN KEY (id, message_type) REFERENCES history(id, "
                      "message_type), FOREIGN KEY (sender_alias) REFERENCES aliases(id))"},
    {"chats", "CREATE TABLE chats (id INTEGER PRIMARY KEY, uuid BLOB NOT NULL UNIQUE)"},
    {"authors", "CREATE TABLE authors (id INTEGER PRIMARY KEY, public_key BLOB NOT NULL UNIQUE)"},
    {"broken_messages", "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY, reason INTEGER NOT "
                        "NULL DEFAULT 0, FOREIGN KEY (id) REFERENCES history(id))"},
    {"system_messages",
     "CREATE TABLE system_messages (id INTEGER PRIMARY KEY, message_type CHAR(1) NOT NULL CHECK "
     "(message_type = 'S'), system_message_type INTEGER NOT NULL, arg1 BLOB, arg2 BLOB, arg3 BLOB, arg4 BLOB, "
     "FOREIGN KEY (id, message_type) REFERENCES history(id, message_type))"},
    {"chat_id_idx", "CREATE INDEX chat_id_idx on history (chat_id)"}};

void DbUtility::createSchemaAtVersion(std::shared_ptr<RawDatabase> db, const std::vector<DbUtility::SqliteMasterEntry>& schema)
{
    QVector<RawDatabase::Query> queries;
    for (auto const& entry : schema) {
        queries += entry.sql;
    }
    QVERIFY(db->execNow(queries));
}

bool DbUtility::SqliteMasterEntry::operator==(const DbUtility::SqliteMasterEntry& rhs) const
{
    return name == rhs.name &&
        sql == rhs.sql;
}

void DbUtility::verifyDb(std::shared_ptr<RawDatabase> db, const std::vector<DbUtility::SqliteMasterEntry>& expectedSql)
{
    QVERIFY(db->execNow(RawDatabase::Query(QStringLiteral(
        "SELECT name, sql FROM sqlite_master;"),
        [&](const QVector<QVariant>& row) {
            const QString tableName = row[0].toString();
            if (row[1].isNull()) {
                // implicit indexes are automatically created for primary key constraints and unique constraints
                // so their existence is already covered by the table creation SQL
                return;
            }
            QString tableSql = row[1].toString();
            // table and column names can be quoted. UPDATE TEABLE automatically quotes the new names, but this
            // has no functional impact on the schema. Strip quotes for comparison so that our created schema
            // matches schema made from UPDATE TABLEs.
            const QString unquotedTableSql = tableSql.remove("\"");
            SqliteMasterEntry entry{tableName, unquotedTableSql};
            QVERIFY(std::find(expectedSql.begin(), expectedSql.end(), entry) != expectedSql.end());
        })));
}
