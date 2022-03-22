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

#include "src/core/toxpk.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/persistence/db/upgrades/dbto11.h"

#include "dbutility/dbutility.h"

#include <QByteArray>
#include <QTemporaryFile>
#include <QTest>

namespace
{
const auto selfPk = ToxPk{QByteArray(32, 0)};
const auto aPk = ToxPk{QByteArray(32, 1)};
const auto bPk = ToxPk{QByteArray(32, 2)};
const auto cPk = ToxPk{QByteArray(32, 3)};
const auto selfName = QStringLiteral("Self");
const auto aName = QStringLiteral("Alice");
const auto bName = QStringLiteral("Bob");
const auto selfAliasId = 1;
const auto aPeerId = 2;
const auto aChatId = 1;
const auto aAliasId = 2;
const auto bPeerId = 3;
const auto bChatId = 2;
const auto bAliasId = 3;
const auto cPeerId = 4;
const auto cChatId = 3;

void appendAddPeersQueries(QVector<RawDatabase::Query>& setupQueries)
{
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO peers (id, public_key) VALUES (1, ?)"),
        {selfPk.toString().toUtf8()}});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO peers (id, public_key) VALUES (2, ?)"),
        {aPk.toString().toUtf8()}});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO peers (id, public_key) VALUES (3, ?)"),
        {bPk.toString().toUtf8()}});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO peers (id, public_key) VALUES (4, ?)"),
        {cPk.toString().toUtf8()}});
}

void appendVerifyChatsQueries(QVector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM chats"),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 3);
        }});

    struct Functor {
        int index = 0;
        const std::vector<QByteArray> chatIds{aPk.getByteArray(), bPk.getByteArray(), cPk.getByteArray()};
        void operator()(const QVector<QVariant>& row) {
            QVERIFY(row[0].toByteArray() == chatIds[index]);
            ++index;
        }
    };

    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT uuid FROM chats"),
        Functor()});
}

void appendVerifyAuthorsQueries(QVector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM authors"),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 3);
        }});

    struct Functor {
        int index = 0;
        const std::vector<QByteArray> chatIds{selfPk.getByteArray(), aPk.getByteArray(), bPk.getByteArray()};
        void operator()(const QVector<QVariant>& row) {
            QVERIFY(row[0].toByteArray() == chatIds[index]);
            ++index;
        }
    };

    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT public_key FROM authors"),
        Functor()});
}

void appendAddAliasesQueries(QVector<RawDatabase::Query>& setupQueries)
{
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO aliases (id, owner, display_name) VALUES (1, 1, ?)"),
        {selfName.toUtf8()}});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO aliases (id, owner, display_name) VALUES (2, 2, ?)"),
        {aName.toUtf8()}});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO aliases (id, owner, display_name) VALUES (3, 3, ?)"),
        {bName.toUtf8()}});
}

void appendVerifyAliasesQueries(QVector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM aliases"),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 3);
        }});

    struct Functor {
        int index = 0;
        const std::vector<QByteArray> names{selfName.toUtf8(), aName.toUtf8(), bName.toUtf8()};
        void operator()(const QVector<QVariant>& row) {
            QVERIFY(row[0].toByteArray() == names[index]);
            ++index;
        }
    };

    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT display_name FROM aliases"),
        Functor()});
}

void appendAddAChatMessagesQueries(QVector<RawDatabase::Query>& setupQueries)
{
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (1, 'T', 0, '%1')").arg(aPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO text_messages (id, message_type, sender_alias, message) VALUES (1, 'T', '%1', ?)").arg(aAliasId),
        {QStringLiteral("Message 1 from A to Self").toUtf8()}});

    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (2, 'T', 0, '%1')").arg(aPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO text_messages (id, message_type, sender_alias, message) VALUES (2, 'T', '%1', ?)").arg(aAliasId),
        {QStringLiteral("Message 2 from A to Self").toUtf8()}});

    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (10, 'F', 0, '%1')").arg(aPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO file_transfers (id, message_type, sender_alias, file_restart_id, "
        "file_name, file_path, file_hash, file_size, direction, file_state) "
        "VALUES(10, 'F', '%1', ?, 'dummy name', 'dummy/path', ?, 1024, 1, 5)").arg(aAliasId),
        {QByteArray(32, 1), QByteArray(32, 2)}});
}

void appendVerifyAChatMessagesQueries(QVector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM history WHERE chat_id = '%1'").arg(aChatId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 3);
        }});
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM text_messages WHERE sender_alias = '%1'").arg(aAliasId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 2);
        }});
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM file_transfers WHERE sender_alias = '%1'").arg(aAliasId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 1);
        }});
}

void appendAddBChatMessagesQueries(QVector<RawDatabase::Query>& setupQueries)
{
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (3, 'T', 0, '%1')").arg(bPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO text_messages (id, message_type, sender_alias, message) VALUES (3, 'T', '%1', ?)").arg(bAliasId),
        {QStringLiteral("Message 1 from B to Self").toUtf8()}});

    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (4, 'T', 0, '%1')").arg(bPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO text_messages (id, message_type, sender_alias, message) VALUES (4, 'T', '%1', ?)").arg(selfAliasId),
        {QStringLiteral("Message 1 from Self to B").toUtf8()}});

    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (5, 'T', 0, '%1')").arg(bPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO text_messages (id, message_type, sender_alias, message) VALUES (5, 'T', '%1', ?)").arg(selfAliasId),
        {QStringLiteral("Pending message 1 from Self to B").toUtf8()}});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO faux_offline_pending (id) VALUES (5)")});

    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (8, 'S', 0, '%1')").arg(bPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO system_messages (id, message_type, system_message_type) VALUES (8, 'S', 1)")});

    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (9, 'F', 0, '%1')").arg(bPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO file_transfers (id, message_type, sender_alias, file_restart_id, "
        "file_name, file_path, file_hash, file_size, direction, file_state) "
        "VALUES(9, 'F', '%1', ?, 'dummy name', 'dummy/path', ?, 1024, 0, 5)").arg(selfAliasId),
        {QByteArray(32, 1), QByteArray(32, 2)}});
}

void appendVerifyBChatMessagesQueries(QVector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM history WHERE chat_id = '%1'").arg(bChatId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 5);
        }});
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM history JOIN text_messages ON "
        "history.id = text_messages.id WHERE chat_id = '%1'").arg(bChatId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 3);
        }});
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM history JOIN faux_offline_pending ON "
        "history.id = faux_offline_pending.id WHERE chat_id = '%1'").arg(bChatId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 1);
        }});
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM file_transfers WHERE sender_alias = '%1'").arg(selfAliasId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 1);
        }});
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM history JOIN system_messages ON "
        "history.id = system_messages.id WHERE chat_id = '%1'").arg(bChatId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 1);
        }});
}

void appendAddCChatMessagesQueries(QVector<RawDatabase::Query>& setupQueries)
{
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (6, 'T', 0, '%1')").arg(cPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO text_messages (id, message_type, sender_alias, message) VALUES (6, 'T', '%1', ?)").arg(selfAliasId),
        {QStringLiteral("Message 1 from Self to B").toUtf8()}});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO broken_messages (id) VALUES (6)")});

    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (7, 'T', 0, '%1')").arg(cPeerId)});
    setupQueries.append(RawDatabase::Query{QStringLiteral(
        "INSERT INTO text_messages (id, message_type, sender_alias, message) VALUES (7, 'T', '%1', ?)").arg(selfAliasId),
        {QStringLiteral("Message 1 from Self to B").toUtf8()}});
}

void appendVerifyCChatMessagesQueries(QVector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM history WHERE chat_id = '%1'").arg(cChatId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 2);
        }});
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM history JOIN broken_messages ON "
        "history.id = broken_messages.id WHERE chat_id = '%1'").arg(cChatId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 1);
        }});
    verifyQueries.append(RawDatabase::Query{QStringLiteral(
        "SELECT COUNT(*) FROM history JOIN text_messages ON "
        "history.id = text_messages.id WHERE chat_id = '%1'").arg(cChatId),
        [&](const QVector<QVariant>& row) {
            QVERIFY(row[0].toLongLong() == 2);
        }});
}

void appendAddHistoryQueries(QVector<RawDatabase::Query>& setupQueries)
{
    appendAddAChatMessagesQueries(setupQueries);
    appendAddBChatMessagesQueries(setupQueries);
    appendAddCChatMessagesQueries(setupQueries);
}

void appendVerifyHistoryQueries(QVector<RawDatabase::Query>& verifyQueries)
{
    appendVerifyAChatMessagesQueries(verifyQueries);
    appendVerifyBChatMessagesQueries(verifyQueries);
    appendVerifyCChatMessagesQueries(verifyQueries);
}
} // namespace

class Test10to11 : public QObject
{
    Q_OBJECT
private slots:
    void test10to11();
private:
    QTemporaryFile testDatabaseFile;
};

void Test10to11::test10to11()
{
    QVERIFY(testDatabaseFile.open());
    testDatabaseFile.close();
    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{testDatabaseFile.fileName(), {}, {}}};
    QVERIFY(db->execNow(RawDatabase::Query{QStringLiteral("PRAGMA foreign_keys = ON;")}));
    createSchemaAtVersion(db, DbUtility::schema10);
    QVector<RawDatabase::Query> setupQueries;
    appendAddPeersQueries(setupQueries);
    appendAddAliasesQueries(setupQueries);
    appendAddHistoryQueries(setupQueries);
    QVERIFY(db->execNow(setupQueries));
    QVERIFY(DbTo11::dbSchema10to11(*db));
    verifyDb(db, DbUtility::schema11);
    QVector<RawDatabase::Query> verifyQueries;
    appendVerifyChatsQueries(verifyQueries);
    appendVerifyAuthorsQueries(verifyQueries);
    appendVerifyAliasesQueries(verifyQueries);
    appendVerifyHistoryQueries(verifyQueries);
    QVERIFY(db->execNow(verifyQueries));
}

QTEST_GUILESS_MAIN(Test10to11)
#include "dbTo11_test.moc"
