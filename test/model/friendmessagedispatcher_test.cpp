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

#include "src/core/icorefriendmessagesender.h"
#include "src/model/friend.h"
#include "src/model/friendmessagedispatcher.h"
#include "src/model/message.h"

#include <QObject>
#include <QtTest/QtTest>

#include <set>
#include <deque>

namespace {
constexpr uint64_t testMaxExtendedMessageSize = 10 * 1024 * 1024;
}

class MockCoreExtPacket : public ICoreExtPacket
{
public:

    MockCoreExtPacket(uint64_t& numSentMessages_, uint64_t& currentReceiptId_)
        : numSentMessages(numSentMessages_)
        , currentReceiptId(currentReceiptId_)
    {}

    uint64_t addExtendedMessage(QString message_) override;

    bool send() override;

    uint64_t& numSentMessages;
    uint64_t& currentReceiptId;
    QDateTime senderTimestamp;
    QString message;
};

uint64_t MockCoreExtPacket::addExtendedMessage(QString message_)
{
    message = message_;
    return currentReceiptId++;
}

bool MockCoreExtPacket::send()
{
    numSentMessages++;
    return true;
}

class MockCoreExtPacketAllocator : public ICoreExtPacketAllocator
{
public:
    std::unique_ptr<ICoreExtPacket> getPacket(uint32_t friendId) override;

    uint64_t numSentMessages;
    uint64_t currentReceiptId;
};

std::unique_ptr<ICoreExtPacket> MockCoreExtPacketAllocator::getPacket(uint32_t friendId)
{
    std::ignore = friendId;
    return std::unique_ptr<MockCoreExtPacket>(new MockCoreExtPacket(numSentMessages, currentReceiptId));
}

class MockFriendMessageSender : public ICoreFriendMessageSender
{
public:
    bool sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt) override;

    bool sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt) override;

    bool canSend = true;
    ReceiptNum receiptNum{0};
    size_t numSentActions = 0;
    size_t numSentMessages = 0;
};

bool MockFriendMessageSender::sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt)
{
    std::ignore = friendId;
    std::ignore = action;
    if (canSend) {
        numSentActions++;
        receipt = receiptNum;
        receiptNum.get() += 1;
    }
    return canSend;
}

bool MockFriendMessageSender::sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt)
{
    std::ignore = friendId;
    std::ignore = message;
    if (canSend) {
        numSentMessages++;
        receipt = receiptNum;
        receiptNum.get() += 1;
    }
    return canSend;
}

class TestFriendMessageDispatcher : public QObject
{
    Q_OBJECT

public:
    TestFriendMessageDispatcher();

private slots:
    void init();
    void testSignals();
    void testMessageSending();
    void testOfflineMessages();
    void testFailedMessage();
    void testNegotiationFailure();
    void testNegotiationSuccess();
    void testOfflineExtensionMessages();
    void testSentMessageExtensionSetReduced();
    void testActionMessagesSplitWithExtensions();

    void onMessageSent(DispatchedMessageId id, Message message)
    {
        auto it = outgoingMessages.find(id);
        QVERIFY(it == outgoingMessages.end());
        outgoingMessages.emplace(id, std::move(message));
    }

    void onMessageComplete(DispatchedMessageId id)
    {
        auto it = outgoingMessages.find(id);
        QVERIFY(it != outgoingMessages.end());
        outgoingMessages.erase(it);
    }

    void onMessageReceived(const ToxPk& sender, Message message)
    {
        std::ignore = sender;
        receivedMessages.push_back(std::move(message));
    }

    void onMessageBroken(DispatchedMessageId id, BrokenMessageReason reason)
    {
        std::ignore = reason;
        brokenMessages.insert(id);
    }

private:
    // All unique_ptrs to make construction/init() easier to manage
    std::unique_ptr<Friend> f;
    std::unique_ptr<MockFriendMessageSender> messageSender;
    std::unique_ptr<MockCoreExtPacketAllocator> coreExtPacketAllocator;
    std::unique_ptr<MessageProcessor::SharedParams> sharedProcessorParams;
    std::unique_ptr<MessageProcessor> messageProcessor;
    std::unique_ptr<FriendMessageDispatcher> friendMessageDispatcher;
    std::map<DispatchedMessageId, Message> outgoingMessages;
    std::set<DispatchedMessageId> brokenMessages;
    std::deque<Message> receivedMessages;
};

TestFriendMessageDispatcher::TestFriendMessageDispatcher() {}

/**
 * @brief Test initialization. Resets all member variables for a fresh test state
 */
void TestFriendMessageDispatcher::init()
{
    f = std::unique_ptr<Friend>(new Friend(0, ToxPk()));
    f->setStatus(Status::Status::Online);
    f->onNegotiationComplete();
    messageSender = std::unique_ptr<MockFriendMessageSender>(new MockFriendMessageSender());
    coreExtPacketAllocator = std::unique_ptr<MockCoreExtPacketAllocator>(new MockCoreExtPacketAllocator());
    sharedProcessorParams =
        std::unique_ptr<MessageProcessor::SharedParams>(new MessageProcessor::SharedParams(tox_max_message_length(), testMaxExtendedMessageSize));

    messageProcessor = std::unique_ptr<MessageProcessor>(new MessageProcessor(*sharedProcessorParams));
    friendMessageDispatcher = std::unique_ptr<FriendMessageDispatcher>(
        new FriendMessageDispatcher(*f, *messageProcessor, *messageSender, *coreExtPacketAllocator));

    connect(friendMessageDispatcher.get(), &FriendMessageDispatcher::messageSent, this,
            &TestFriendMessageDispatcher::onMessageSent);
    connect(friendMessageDispatcher.get(), &FriendMessageDispatcher::messageComplete, this,
            &TestFriendMessageDispatcher::onMessageComplete);
    connect(friendMessageDispatcher.get(), &FriendMessageDispatcher::messageReceived, this,
            &TestFriendMessageDispatcher::onMessageReceived);
    connect(friendMessageDispatcher.get(), &FriendMessageDispatcher::messageBroken, this,
            &TestFriendMessageDispatcher::onMessageBroken);

    outgoingMessages = std::map<DispatchedMessageId, Message>();
    receivedMessages = std::deque<Message>();
    brokenMessages = std::set<DispatchedMessageId>();
}

/**
 * @brief Tests that the signals emitted by the dispatcher are all emitted at the correct times
 */
void TestFriendMessageDispatcher::testSignals()
{
    auto startReceiptNum = messageSender->receiptNum;
    auto sentIds = friendMessageDispatcher->sendMessage(false, "test");
    auto endReceiptNum = messageSender->receiptNum;

    // We should have received some message ids in our callbacks
    QVERIFY(sentIds.first == sentIds.second);
    QVERIFY(outgoingMessages.find(sentIds.first) != outgoingMessages.end());
    QVERIFY(startReceiptNum.get() != endReceiptNum.get());
    QVERIFY(outgoingMessages.size() == 1);

    QVERIFY(outgoingMessages.begin()->second.isAction == false);
    QVERIFY(outgoingMessages.begin()->second.content == "test");

    for (auto i = startReceiptNum; i < endReceiptNum; ++i.get()) {
        friendMessageDispatcher->onReceiptReceived(i);
    }

    // If our completion ids were hooked up right this should be empty
    QVERIFY(outgoingMessages.empty());

    // If signals are emitted correctly we should have one message in our received message buffer
    QVERIFY(receivedMessages.empty());
    friendMessageDispatcher->onMessageReceived(false, "test2");

    QVERIFY(!receivedMessages.empty());
    QVERIFY(receivedMessages.front().isAction == false);
    QVERIFY(receivedMessages.front().content == "test2");
}

/**
 * @brief Tests that sent messages actually go through to core
 */
void TestFriendMessageDispatcher::testMessageSending()
{
    friendMessageDispatcher->sendMessage(false, "Test");

    QVERIFY(messageSender->numSentMessages == 1);
    QVERIFY(messageSender->numSentActions == 0);

    friendMessageDispatcher->sendMessage(true, "Test");

    QVERIFY(messageSender->numSentMessages == 1);
    QVERIFY(messageSender->numSentActions == 1);
}

/**
 * @brief Tests that messages dispatched while a friend is offline are sent later
 */
void TestFriendMessageDispatcher::testOfflineMessages()
{
    f->setStatus(Status::Status::Offline);
    auto firstReceipt = messageSender->receiptNum;

    friendMessageDispatcher->sendMessage(false, "test");
    friendMessageDispatcher->sendMessage(false, "test2");
    friendMessageDispatcher->sendMessage(true, "test3");

    QVERIFY(messageSender->numSentActions == 0);
    QVERIFY(messageSender->numSentMessages == 0);
    QVERIFY(outgoingMessages.size() == 3);

    f->setStatus(Status::Status::Online);
    f->onNegotiationComplete();

    QVERIFY(messageSender->numSentActions == 1);
    QVERIFY(messageSender->numSentMessages == 2);
    QVERIFY(outgoingMessages.size() == 3);

    auto lastReceipt = messageSender->receiptNum;
    for (auto i = firstReceipt; i < lastReceipt; ++i.get()) {
        friendMessageDispatcher->onReceiptReceived(i);
    }

    QVERIFY(messageSender->numSentActions == 1);
    QVERIFY(messageSender->numSentMessages == 2);
    QVERIFY(outgoingMessages.size() == 0);
}

/**
 * @brief Tests that messages that failed to send due to toxcore are resent later
 */
void TestFriendMessageDispatcher::testFailedMessage()
{
    messageSender->canSend = false;

    friendMessageDispatcher->sendMessage(false, "test");

    QVERIFY(messageSender->numSentMessages == 0);

    messageSender->canSend = true;
    f->setStatus(Status::Status::Offline);
    f->setStatus(Status::Status::Online);
    f->onNegotiationComplete();

    QVERIFY(messageSender->numSentMessages == 1);
}

void TestFriendMessageDispatcher::testNegotiationFailure()
{
    f->setStatus(Status::Status::Offline);
    f->setStatus(Status::Status::Online);

    QVERIFY(f->getStatus() == Status::Status::Negotiating);

    friendMessageDispatcher->sendMessage(false, "test");

    QVERIFY(messageSender->numSentMessages == 0);

    f->onNegotiationComplete();

    QVERIFY(messageSender->numSentMessages == 1);
}

void TestFriendMessageDispatcher::testNegotiationSuccess()
{
    f->setStatus(Status::Status::Offline);
    f->setStatus(Status::Status::Online);

    f->setExtendedMessageSupport(true);
    f->onNegotiationComplete();

    friendMessageDispatcher->sendMessage(false, "test");

    QVERIFY(coreExtPacketAllocator->numSentMessages == 1);

    friendMessageDispatcher->sendMessage(false, "test");
    QVERIFY(coreExtPacketAllocator->numSentMessages == 2);
    QVERIFY(messageSender->numSentMessages == 0);
}

void TestFriendMessageDispatcher::testOfflineExtensionMessages()
{
    f->setStatus(Status::Status::Offline);

    auto requiredExtensions = ExtensionSet();
    requiredExtensions[ExtensionType::messages] = true;

    friendMessageDispatcher->sendExtendedMessage("Test", requiredExtensions);

    f->setStatus(Status::Status::Online);
    f->setExtendedMessageSupport(true);
    f->onNegotiationComplete();

    // Ensure that when our friend came online with the desired extensions we
    // were able to send them our message over the extended message path
    QVERIFY(coreExtPacketAllocator->numSentMessages == 1);

    f->setStatus(Status::Status::Offline);

    friendMessageDispatcher->sendExtendedMessage("Test", requiredExtensions);

    f->setStatus(Status::Status::Online);
    f->setExtendedMessageSupport(false);
    f->onNegotiationComplete();

    // Here we want to make sure that when they do _not_ support extensions
    // we discard the message instead of attempting to send it over either
    // channel
    QVERIFY(coreExtPacketAllocator->numSentMessages == 1);
    QVERIFY(messageSender->numSentMessages == 0);
}

void TestFriendMessageDispatcher::testSentMessageExtensionSetReduced()
{
    f->setStatus(Status::Status::Online);
    f->setExtendedMessageSupport(true);
    f->onNegotiationComplete();

    friendMessageDispatcher->sendMessage(false, "Test");

    f->setStatus(Status::Status::Offline);
    f->setStatus(Status::Status::Online);
    f->setExtendedMessageSupport(false);
    f->onNegotiationComplete();

    // Ensure that when we reduce our extension set we correctly emit the
    // "messageBroken" signal
    QVERIFY(brokenMessages.size() == 1);
}

void TestFriendMessageDispatcher::testActionMessagesSplitWithExtensions()
{
    f->setStatus(Status::Status::Online);
    f->setExtendedMessageSupport(true);
    f->onNegotiationComplete();

    auto reallyLongMessage = QString("a");

    for (uint64_t i = 0; i < testMaxExtendedMessageSize + 50; ++i) {
        reallyLongMessage += QString().number(i);
    }

    friendMessageDispatcher->sendMessage(true, reallyLongMessage);

    QVERIFY(coreExtPacketAllocator->numSentMessages == 0);
    QVERIFY(messageSender->numSentMessages == 0);
    QVERIFY(messageSender->numSentActions > 1);
}

QTEST_GUILESS_MAIN(TestFriendMessageDispatcher)
#include "friendmessagedispatcher_test.moc"
