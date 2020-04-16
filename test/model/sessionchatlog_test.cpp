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

#include "src/model/ichatlog.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/sessionchatlog.h"

#include <QtTest/QtTest>

namespace {
Message createMessage(const QString& content)
{
    Message message;
    message.content = content;
    message.isAction = false;
    message.timestamp = QDateTime::currentDateTime();
    return message;
}

class MockCoreIdHandler : public ICoreIdHandler
{
public:
    ToxId getSelfId() const override
    {
        std::terminate();
        return ToxId();
    }

    ToxPk getSelfPublicKey() const override
    {
        static uint8_t id[TOX_PUBLIC_KEY_SIZE] = {5};
        return ToxPk(id);
    }

    QString getUsername() const override
    {
        std::terminate();
        return QString();
    }
};
} // namespace

class TestSessionChatLog : public QObject
{
    Q_OBJECT

public:
    TestSessionChatLog(){}

private slots:
    void init();

    void testSanity();

private:
    MockCoreIdHandler idHandler;
    std::unique_ptr<SessionChatLog> chatLog;
};

/**
 * @brief Test initialiation, resets the chatlog
 */
void TestSessionChatLog::init()
{
    chatLog = std::unique_ptr<SessionChatLog>(new SessionChatLog(idHandler));
}

/**
 * @brief Quick sanity test that the chatlog is working as expected. Tests basic insertion, retrieval, and searching of messages
 */
void TestSessionChatLog::testSanity()
{
    /* ChatLogIdx(0) */ chatLog->onMessageSent(DispatchedMessageId(0), createMessage("test"));
    /* ChatLogIdx(1) */ chatLog->onMessageSent(DispatchedMessageId(1), createMessage("test test"));
    /* ChatLogIdx(2) */ chatLog->onMessageReceived(ToxPk(), createMessage("test2"));
    /* ChatLogIdx(3) */ chatLog->onFileUpdated(ToxPk(), ToxFile());
    /* ChatLogIdx(4) */ chatLog->onMessageSent(DispatchedMessageId(2), createMessage("test3"));
    /* ChatLogIdx(5) */ chatLog->onMessageSent(DispatchedMessageId(3), createMessage("test4"));
    /* ChatLogIdx(6) */ chatLog->onMessageSent(DispatchedMessageId(4), createMessage("test"));
    /* ChatLogIdx(7) */ chatLog->onMessageReceived(ToxPk(), createMessage("test5"));

    QVERIFY(chatLog->getNextIdx() == ChatLogIdx(8));
    QVERIFY(chatLog->at(ChatLogIdx(3)).getContentType() == ChatLogItem::ContentType::fileTransfer);
    QVERIFY(chatLog->at(ChatLogIdx(7)).getContentType() == ChatLogItem::ContentType::message);

    auto searchPos = SearchPos{ChatLogIdx(1), 0};
    auto searchResult = chatLog->searchForward(searchPos, "test", ParameterSearch());

    QVERIFY(searchResult.found);
    QVERIFY(searchResult.len == 4);
    QVERIFY(searchResult.pos.logIdx == ChatLogIdx(1));
    QVERIFY(searchResult.start == 0);

    searchPos = searchResult.pos;
    searchResult = chatLog->searchForward(searchPos, "test", ParameterSearch());

    QVERIFY(searchResult.found);
    QVERIFY(searchResult.len == 4);
    QVERIFY(searchResult.pos.logIdx == ChatLogIdx(1));
    QVERIFY(searchResult.start == 5);

    searchPos = searchResult.pos;
    searchResult = chatLog->searchForward(searchPos, "test", ParameterSearch());

    QVERIFY(searchResult.found);
    QVERIFY(searchResult.len == 4);
    QVERIFY(searchResult.pos.logIdx == ChatLogIdx(2));
    QVERIFY(searchResult.start == 0);

    searchPos = searchResult.pos;
    searchResult = chatLog->searchBackward(searchPos, "test", ParameterSearch());

    QVERIFY(searchResult.found);
    QVERIFY(searchResult.len == 4);
    QVERIFY(searchResult.pos.logIdx == ChatLogIdx(1));
    QVERIFY(searchResult.start == 5);
}

QTEST_GUILESS_MAIN(TestSessionChatLog)
#include "sessionchatlog_test.moc"
