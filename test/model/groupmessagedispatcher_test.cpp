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

#include "src/core/icoregroupmessagesender.h"
#include "src/model/group.h"
#include "src/model/groupmessagedispatcher.h"
#include "src/model/message.h"
#include "src/persistence/settings.h"

#include <QObject>
#include <QtTest/QtTest>

#include <set>
#include <deque>


class MockGroupMessageSender : public ICoreGroupMessageSender
{
public:
    void sendGroupAction(int groupId, const QString& action) override
    {
        numSentActions++;
    }

    void sendGroupMessage(int groupId, const QString& message) override
    {
        numSentMessages++;
    }

    size_t numSentActions = 0;
    size_t numSentMessages = 0;
};

/**
 * Mock 1 peer at group number 0
 */
class MockGroupQuery : public ICoreGroupQuery
{
public:
    GroupId getGroupPersistentId(uint32_t groupNumber) const override
    {
        return GroupId();
    }

    uint32_t getGroupNumberPeers(int groupId) const override
    {
        if (emptyGroup) {
            return 1;
        }

        return 2;
    }

    QString getGroupPeerName(int groupId, int peerId) const override
    {
        return QString("peer") + peerId;
    }

    ToxPk getGroupPeerPk(int groupId, int peerId) const override
    {
        uint8_t id[TOX_PUBLIC_KEY_SIZE] = {static_cast<uint8_t>(peerId)};
        return ToxPk(id);
    }

    QStringList getGroupPeerNames(int groupId) const override
    {
        if (emptyGroup) {
            return QStringList({QString("me")});
        }
        return QStringList({QString("me"), QString("other")});
    }

    bool getGroupAvEnabled(int groupId) const override
    {
        return false;
    }

    void setAsEmptyGroup()
    {
        emptyGroup = true;
    }

    void setAsFunctionalGroup()
    {
        emptyGroup = false;
    }

private:
    bool emptyGroup = false;
};

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
        static uint8_t id[TOX_PUBLIC_KEY_SIZE] = {0};
        return ToxPk(id);
    }

    QString getUsername() const override
    {
        return "me";
    }
};

class MockGroupSettings : public IGroupSettings
{
public:
    QStringList getBlackList() const override
    {
        return blacklist;
    }

    void setBlackList(const QStringList& blist) override
    {
        blacklist = blist;
    }

    bool getGroupAlwaysNotify() const override
    {
        return false;
    }

    void setGroupAlwaysNotify(bool newValue) override {}

private:
    QStringList blacklist;
};

class TestGroupMessageDispatcher : public QObject
{
    Q_OBJECT

public:
    TestGroupMessageDispatcher();

private slots:
    void init();
    void testSignals();
    void testMessageSending();
    void testEmptyGroup();
    void testSelfReceive();
    void testBlacklist();

    void onMessageSent(DispatchedMessageId id, Message message)
    {
        auto it = outgoingMessages.find(id);
        QVERIFY(it == outgoingMessages.end());
        outgoingMessages.emplace(id);
        sentMessages.push_back(std::move(message));
    }

    void onMessageComplete(DispatchedMessageId id)
    {
        auto it = outgoingMessages.find(id);
        QVERIFY(it != outgoingMessages.end());
        outgoingMessages.erase(it);
    }

    void onMessageReceived(const ToxPk& sender, Message message)
    {
        receivedMessages.push_back(std::move(message));
    }

private:
    // All unique_ptrs to make construction/init() easier to manage
    std::unique_ptr<MockGroupSettings> groupSettings;
    std::unique_ptr<MockGroupQuery> groupQuery;
    std::unique_ptr<MockCoreIdHandler> coreIdHandler;
    std::unique_ptr<Group> g;
    std::unique_ptr<MockGroupMessageSender> messageSender;
    std::unique_ptr<MessageProcessor::SharedParams> sharedProcessorParams;
    std::unique_ptr<MessageProcessor> messageProcessor;
    std::unique_ptr<GroupMessageDispatcher> groupMessageDispatcher;
    std::set<DispatchedMessageId> outgoingMessages;
    std::deque<Message> sentMessages;
    std::deque<Message> receivedMessages;
};

TestGroupMessageDispatcher::TestGroupMessageDispatcher() {}

/**
 * @brief Test initialization. Resets all members to initial state
 */
void TestGroupMessageDispatcher::init()
{
    groupSettings = std::unique_ptr<MockGroupSettings>(new MockGroupSettings());
    groupQuery = std::unique_ptr<MockGroupQuery>(new MockGroupQuery());
    coreIdHandler = std::unique_ptr<MockCoreIdHandler>(new MockCoreIdHandler());
    g = std::unique_ptr<Group>(
        new Group(0, GroupId(), "TestGroup", false, "me", *groupQuery, *coreIdHandler));
    messageSender = std::unique_ptr<MockGroupMessageSender>(new MockGroupMessageSender());
    sharedProcessorParams =
        std::unique_ptr<MessageProcessor::SharedParams>(new MessageProcessor::SharedParams());
    messageProcessor = std::unique_ptr<MessageProcessor>(new MessageProcessor(*sharedProcessorParams));
    groupMessageDispatcher = std::unique_ptr<GroupMessageDispatcher>(
        new GroupMessageDispatcher(*g, *messageProcessor, *coreIdHandler, *messageSender,
                                   *groupSettings));

    connect(groupMessageDispatcher.get(), &GroupMessageDispatcher::messageSent, this,
            &TestGroupMessageDispatcher::onMessageSent);
    connect(groupMessageDispatcher.get(), &GroupMessageDispatcher::messageComplete, this,
            &TestGroupMessageDispatcher::onMessageComplete);
    connect(groupMessageDispatcher.get(), &GroupMessageDispatcher::messageReceived, this,
            &TestGroupMessageDispatcher::onMessageReceived);

    outgoingMessages = std::set<DispatchedMessageId>();
    sentMessages = std::deque<Message>();
    receivedMessages = std::deque<Message>();
}

/**
 * @brief Tests that the signals emitted by the dispatcher are all emitted at the correct times
 */
void TestGroupMessageDispatcher::testSignals()
{
    groupMessageDispatcher->sendMessage(false, "test");

    // For groups we pair our sent and completed signals since we have no receiver reports
    QVERIFY(outgoingMessages.size() == 0);
    QVERIFY(!sentMessages.empty());
    QVERIFY(sentMessages.front().isAction == false);
    QVERIFY(sentMessages.front().content == "test");

    // If signals are emitted correctly we should have one message in our received message buffer
    QVERIFY(receivedMessages.empty());
    groupMessageDispatcher->onMessageReceived(ToxPk(), false, "test2");

    QVERIFY(!receivedMessages.empty());
    QVERIFY(receivedMessages.front().isAction == false);
    QVERIFY(receivedMessages.front().content == "test2");
}

/**
 * @brief Tests that sent messages actually go through to core
 */
void TestGroupMessageDispatcher::testMessageSending()
{
    groupMessageDispatcher->sendMessage(false, "Test");

    QVERIFY(messageSender->numSentMessages == 1);
    QVERIFY(messageSender->numSentActions == 0);

    groupMessageDispatcher->sendMessage(true, "Test");

    QVERIFY(messageSender->numSentMessages == 1);
    QVERIFY(messageSender->numSentActions == 1);
}

/**
 * @brief Tests that if we are the only member in a group we do _not_ send messages to core. Toxcore
 * isn't too happy if we send messages and we're the only one in the group
 */
void TestGroupMessageDispatcher::testEmptyGroup()
{
    groupQuery->setAsEmptyGroup();
    g->regeneratePeerList();

    groupMessageDispatcher->sendMessage(false, "Test");
    groupMessageDispatcher->sendMessage(true, "Test");

    QVERIFY(messageSender->numSentMessages == 0);
    QVERIFY(messageSender->numSentActions == 0);
}

/**
 * @brief Tests that we do not emit any signals if we receive a message from ourself. Toxcore will send us back messages we sent
 */
void TestGroupMessageDispatcher::testSelfReceive()
{
    uint8_t selfId[TOX_PUBLIC_KEY_SIZE] = {0};
    groupMessageDispatcher->onMessageReceived(ToxPk(selfId), false, "Test");
    QVERIFY(receivedMessages.size() == 0);

    uint8_t id[TOX_PUBLIC_KEY_SIZE] = {1};
    groupMessageDispatcher->onMessageReceived(ToxPk(id), false, "Test");
    QVERIFY(receivedMessages.size() == 1);
}

/**
 * @brief Tests that messages from blacklisted peers do not get propogated from the dispatcher
 */
void TestGroupMessageDispatcher::testBlacklist()
{
    uint8_t id[TOX_PUBLIC_KEY_SIZE] = {1};
    auto otherPk = ToxPk(id);
    groupMessageDispatcher->onMessageReceived(otherPk, false, "Test");
    QVERIFY(receivedMessages.size() == 1);

    groupSettings->setBlackList({otherPk.toString()});
    groupMessageDispatcher->onMessageReceived(otherPk, false, "Test");
    QVERIFY(receivedMessages.size() == 1);
}

// Cannot be guiless due to a settings instance in GroupMessageDispatcher
QTEST_GUILESS_MAIN(TestGroupMessageDispatcher)
#include "groupmessagedispatcher_test.moc"
