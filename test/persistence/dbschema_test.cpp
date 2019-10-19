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

#include "src/persistence/db/rawdatabase.h"
// normally we should only test public API instead of implementation,  but there's no reason to expose db schema
// upgrade externally, and the complexity of each version upgrade benefits from being individually testable
#include "src/persistence/history.cpp"

#include <QtTest/QtTest>
#include <QString>

#include <memory>

class TestDbSchema : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void testCreation();
    void testIsNewDb();
    void test0to1();
    void test1to2();
    void cleanupTestCase();
private:
    bool initSucess{false};
    void createSchemaAtVersion(std::shared_ptr<RawDatabase>, const QMap<QString, QString>& schema);
    void verifyDb(std::shared_ptr<RawDatabase> db, const QMap<QString, QString>& expectedSql);
};

const QString testFileList[] = {
    "testCreation.db",
    "testIsNewDbTrue.db",
    "testIsNewDbFalse.db",
    "test0to1.db",
    "test1to2.db"
};

const QMap<QString, QString> schema0 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL)"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"}
};

// added file transfer history
const QMap<QString, QString> schema1 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
    {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER)"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"}
};

// move stuck faux offline messages do a table of "broken" messages
const QMap<QString, QString> schema2 {
    {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
    {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
    {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
    {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER)"},
    {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"},
    {"broken_messages", "CREATE TABLE broken_messages (id INTEGER PRIMARY KEY)"}
};

void TestDbSchema::initTestCase()
{
    for (const auto& path : testFileList) {
        QVERIFY(!QFileInfo{path}.exists());
    }
    initSucess = true;
}

void TestDbSchema::cleanupTestCase()
{
    if (!initSucess) {
        qWarning() << "init failed, skipping cleanup to avoid loss of data";
        return;
    }
    for (const auto& path : testFileList) {
        QFile::remove(path);
    }
}

void TestDbSchema::verifyDb(std::shared_ptr<RawDatabase> db, const QMap<QString, QString>& expectedSql)
{
    QVERIFY(db->execNow(RawDatabase::Query(QStringLiteral(
        "SELECT name, sql FROM sqlite_master "
        "WHERE type='table';"),
        [&](const QVector<QVariant>& row) {
            const QString tableName = row[0].toString();
            QString tableSql = row[1].toString();
            QVERIFY(expectedSql.contains(tableName));
            // table and column names can be quoted. UPDATE TEABLE automatically quotes the new names, but this
            // has no functional impact on the schema. Strip quotes for comparison so that our created schema
            // matches schema made from UPDATE TABLEs.
            const QString unquotedTableSql = tableSql.remove("\"");
            QVERIFY(expectedSql.value(tableName) == unquotedTableSql);
        })));
}

void TestDbSchema::createSchemaAtVersion(std::shared_ptr<RawDatabase> db, const QMap<QString, QString>& schema)
{
    QVector<RawDatabase::Query> queries;
    for (auto const& tableCreation : schema.values()) {
        queries += tableCreation;
    }
    QVERIFY(db->execNow(queries));
}

void TestDbSchema::testCreation()
{
    QVector<RawDatabase::Query> queries;
    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{"testCreation.db", {}, {}}};
    QVERIFY(createCurrentSchema(*db));
    verifyDb(db, schema2);
}

void TestDbSchema::testIsNewDb()
{
    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{"testIsNewDbTrue.db", {}, {}}};
    bool success = false;
    bool newDb = isNewDb(db, success);
    QVERIFY(success);
    QVERIFY(newDb == true);
    db = std::shared_ptr<RawDatabase>{new RawDatabase{"testIsNewDbFalse.db", {}, {}}};
    createSchemaAtVersion(db, schema0);
    newDb = isNewDb(db, success);
    QVERIFY(success);
    QVERIFY(newDb == false);
}

void TestDbSchema::test0to1()
{
    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{"test0to1.db", {}, {}}};
    createSchemaAtVersion(db, schema0);
    QVERIFY(dbSchema0to1(*db));
    verifyDb(db, schema1);
}

void TestDbSchema::test1to2()
{
    /*
    Due to a long standing bug, faux offline message have been able to become stuck
    going back years. Because of recent fixes to history loading, faux offline
    messages will correctly all be sent on connection, but this causes an issue of
    long stuck messages suddenly being delivered to a friend, out of context,
    creating a confusing interaction. To work around this, this upgrade moves any
    faux offline messages in a chat that are older than the last successfully
    delivered message, indicating they were stuck, to a new table,
    `broken_messages`, preventing them from ever being sent in the future.

    https://github.com/qTox/qTox/issues/5776
    */

    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{"test1to2.db", {}, {}}};
    createSchemaAtVersion(db, schema1);

    const QString myPk = "AC18841E56CCDEE16E93E10E6AB2765BE54277D67F1372921B5B418A6B330D3D";
    const QString friend1Pk = "FE34BC6D87B66E958C57BBF205F9B79B62BE0AB8A4EFC1F1BB9EC4D0D8FB0663";
    const QString friend2Pk = "2A1CBCE227549459C0C20F199DB86AD9BCC436D35BAA1825FFD4B9CA3290D200";

    QVector<RawDatabase::Query> queries;
    queries += QString("INSERT INTO peers (id, public_key) VALUES (%1, '%2')").arg(0).arg(myPk);
    queries += QString("INSERT INTO peers (id, public_key) VALUES (%1, '%2')").arg(1).arg(friend1Pk);
    queries += QString("INSERT INTO peers (id, public_key) VALUES (%1, '%2')").arg(2).arg(friend2Pk);

    // friend 1
    // first message in chat is pending - but the second is delivered. This message is "broken"
    queries += RawDatabase::Query{
        "INSERT INTO history (id, timestamp, chat_id, message, sender_alias) VALUES (1, 1, 1, ?, 0)",
        {"first message in chat, pending and stuck"}};
    queries += {"INSERT INTO faux_offline_pending (id) VALUES ("
                                        "    last_insert_rowid()"
                                        ");"};
    // second message is delivered, causing the first to be considered broken
    queries += RawDatabase::Query{
        "INSERT INTO history (id, timestamp, chat_id, message, sender_alias) VALUES (2, 2, 1, ?, 0)",
        {"second message in chat, delivered"}};

    // third message is pending - this is a normal pending message. It should be untouched.
    queries += RawDatabase::Query{
        "INSERT INTO history (id, timestamp, chat_id, message, sender_alias) VALUES (3, 3, 1, ?, 0)",
        {"third message in chat, pending"}};
    queries += {"INSERT INTO faux_offline_pending (id) VALUES ("
                                        "    last_insert_rowid()"
                                        ");"};

    // friend 2
    // first message is delivered.
    queries += RawDatabase::Query{
        "INSERT INTO history (id, timestamp, chat_id, message, sender_alias) VALUES (4, 4, 2, ?, 2)",
        {"first message by friend in chat, delivered"}};

    // second message is also delivered.
    queries += RawDatabase::Query{
        "INSERT INTO history (id, timestamp, chat_id, message, sender_alias) VALUES (5, 5, 2, ?, 0)",
        {"first message by us in chat, delivered"}};

    // third message is pending, but not broken since there are no delivered messages after it.
    queries += RawDatabase::Query{
        "INSERT INTO history (id, timestamp, chat_id, message, sender_alias) VALUES (6, 6, 2, ?, 0)",
        {"last message in chat, by us, pending"}};
    queries += {"INSERT INTO faux_offline_pending (id) VALUES ("
                                        "    last_insert_rowid()"
                                        ");"};

    QVERIFY(db->execNow(queries));
    QVERIFY(dbSchema1to2(*db));
    verifyDb(db, schema2);

    long brokenCount = -1;
    RawDatabase::Query brokenCountQuery = {"SELECT COUNT(*) FROM broken_messages;", [&](const QVector<QVariant>& row) {
        brokenCount = row[0].toLongLong();
    }};
    QVERIFY(db->execNow(brokenCountQuery));
    QVERIFY(brokenCount == 1); // only friend 1's first message is "broken"

    int fauxOfflineCount = -1;
    RawDatabase::Query fauxOfflineCountQuery = {"SELECT COUNT(*) FROM faux_offline_pending;", [&](const QVector<QVariant>& row) {
        fauxOfflineCount = row[0].toLongLong();
    }};
    QVERIFY(db->execNow(fauxOfflineCountQuery));
    // both friend 1's third message and friend 2's third message should still be pending.
    //The broken message should no longer be pending.
    QVERIFY(fauxOfflineCount == 2);

    int totalHisoryCount = -1;
    RawDatabase::Query totalHistoryCountQuery = {"SELECT COUNT(*) FROM history;", [&](const QVector<QVariant>& row) {
        totalHisoryCount = row[0].toLongLong();
    }};
    QVERIFY(db->execNow(totalHistoryCountQuery));
    QVERIFY(totalHisoryCount == 6); // all messages should still be in history.
}

QTEST_GUILESS_MAIN(TestDbSchema)
#include "dbschema_test.moc"
