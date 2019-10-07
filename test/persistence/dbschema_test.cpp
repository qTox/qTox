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
    void createSchemaAtVersion(std::shared_ptr<RawDatabase>, const QMap<QString, QString>& schema);
    void verifyDb(std::shared_ptr<RawDatabase> db, const QMap<QString, QString>& expectedSql);
};

const QString testFileList[] = {
    "testCreation.db",
    "testIsNewDbTrue.db",
    "testIsNewDbFalse.db",
    "test0to1.db"
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
    QVERIFY(createCurrentSchema(db));
    verifyDb(db, schema1);
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
    QVERIFY(dbSchema0to1(db));
    verifyDb(db, schema1);
}

QTEST_GUILESS_MAIN(TestDbSchema)
#include "dbschema_test.moc"
