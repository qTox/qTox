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
    void cleanupTestCase();
private:
    bool initSucess{false};
    void verifyDb(std::shared_ptr<RawDatabase> db, const QMap<QString, QString>& expectedSql);
};

const QString testFileList[] = {
    "testCreation.db",
    "testIsNewDbTrue.db",
    "testIsNewDbFalse.db",
    "test0to1.db"
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
    QVERIFY(db->execNow(RawDatabase::Query(QStringLiteral("SELECT name, sql FROM sqlite_master "
        "WHERE type='table' "
        "ORDER BY name;"),
        [&](const QVector<QVariant>& row) {
            const QString tableName = row[0].toString();
            const QString tableSql = row[1].toString();
            QVERIFY(expectedSql.contains(tableName));
            QVERIFY(expectedSql.value(tableName) == tableSql);
        })));
}

void TestDbSchema::testCreation()
{
    QVector<RawDatabase::Query> queries;
    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{"testCreation.db", {}, {}}};
    generateCurrentSchema(queries);
    QVERIFY(db->execNow(queries));
    const QMap<QString, QString> expectedSql {
        {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
        {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
        {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
        {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER)"},
        {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"}
    };
    verifyDb(db, expectedSql);
}

void TestDbSchema::testIsNewDb()
{
    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{"testIsNewDbTrue.db", {}, {}}};
    QVERIFY(isNewDb(db) == true);
    db = std::shared_ptr<RawDatabase>{new RawDatabase{"testIsNewDbFalse.db", {}, {}}};
    QVector<RawDatabase::Query> queries;
    generateCurrentSchema(queries);
    QVERIFY(db->execNow(queries));
    QVERIFY(isNewDb(db) == false);
}

void TestDbSchema::test0to1()
{
    const QMap<QString, QString> expectedSql {
        {"aliases", "CREATE TABLE aliases (id INTEGER PRIMARY KEY, owner INTEGER, display_name BLOB NOT NULL, UNIQUE(owner, display_name))"},
        {"faux_offline_pending", "CREATE TABLE faux_offline_pending (id INTEGER PRIMARY KEY)"},
        {"file_transfers", "CREATE TABLE file_transfers (id INTEGER PRIMARY KEY, chat_id INTEGER NOT NULL, file_restart_id BLOB NOT NULL, file_name BLOB NOT NULL, file_path BLOB NOT NULL, file_hash BLOB NOT NULL, file_size INTEGER NOT NULL, direction INTEGER NOT NULL, file_state INTEGER NOT NULL)"},
        {"history", "CREATE TABLE history (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL, message BLOB NOT NULL, file_id INTEGER)"},
        {"peers", "CREATE TABLE peers (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE)"}
    };
    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{"test0to1.db", {}, {}}};
    QVector<RawDatabase::Query> queries;
    queries += RawDatabase::Query(QStringLiteral(
        "CREATE TABLE peers "
        "(id INTEGER PRIMARY KEY, "
        "public_key TEXT NOT NULL UNIQUE);"
        "CREATE TABLE aliases "
        "(id INTEGER PRIMARY KEY, "
        "owner INTEGER, "
        "display_name BLOB NOT NULL, "
        "UNIQUE(owner, display_name));"
        "CREATE TABLE history "
        "(id INTEGER PRIMARY KEY, "
        "timestamp INTEGER NOT NULL, "
        "chat_id INTEGER NOT NULL, "
        "sender_alias INTEGER NOT NULL, "
        "message BLOB NOT NULL);"
        "CREATE TABLE faux_offline_pending "
        "(id INTEGER PRIMARY KEY);"));
    QVERIFY(db->execNow(queries));
    queries.clear();
    dbSchema0to1(db, queries);
    QVERIFY(db->execNow(queries));
    verifyDb(db, expectedSql);
}

QTEST_GUILESS_MAIN(TestDbSchema)
#include "dbschema_test.moc"
