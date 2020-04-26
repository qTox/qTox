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

#include "src/core/core.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/persistence/offlinemsgengine.h"

#include <QtTest/QtTest>

struct MockFriendMessageSender : public QObject, public ICoreFriendMessageSender
{
    Q_OBJECT
public:
    MockFriendMessageSender(Friend* f)
        : f(f){}
    bool sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt) override
    {
        return false;
    }
    bool sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt) override
    {
        if (Status::isOnline(f->getStatus())) {
            receipt.get() = receiptNum++;
            if (!dropReceipts) {
                msgs.push_back(message);
                emit receiptReceived(receipt);
            }
            numMessagesSent++;
        } else {
            numMessagesFailed++;
        }
        return Status::isOnline(f->getStatus());
    }

signals:
    void receiptReceived(ReceiptNum receipt);

public:
    Friend* f;
    bool dropReceipts = false;
    size_t numMessagesSent = 0;
    size_t numMessagesFailed = 0;
    int receiptNum = 0;
    std::vector<QString> msgs;
};

class TestOfflineMsgEngine : public QObject
{
    Q_OBJECT

private slots:
    void testReceiptResolution();
    void testOfflineFriend();
    void testSentUnsentCoordination();
    void testCallback();
};

class OfflineMsgEngineFixture
{
public:
    OfflineMsgEngineFixture()
        : f(0, ToxPk(QByteArray(32, 0)))
        , friendMessageSender(&f)
        , offlineMsgEngine(&f, &friendMessageSender)
    {
        f.setStatus(Status::Status::Online);
        QObject::connect(&friendMessageSender, &MockFriendMessageSender::receiptReceived,
                         &offlineMsgEngine, &OfflineMsgEngine::onReceiptReceived);
    }

    Friend f;
    MockFriendMessageSender friendMessageSender;
    OfflineMsgEngine offlineMsgEngine;
};

void completionFn() {}

void TestOfflineMsgEngine::testReceiptResolution()
{
    OfflineMsgEngineFixture fixture;

    Message msg{false, QString(), QDateTime()};

    ReceiptNum receipt;
    fixture.friendMessageSender.sendMessage(0, msg.content, receipt);
    fixture.offlineMsgEngine.addSentMessage(receipt, msg, completionFn);

    // We should have no offline messages to deliver if we resolved our receipt
    // correctly
    fixture.offlineMsgEngine.deliverOfflineMsgs();
    fixture.offlineMsgEngine.deliverOfflineMsgs();
    fixture.offlineMsgEngine.deliverOfflineMsgs();

    QVERIFY(fixture.friendMessageSender.numMessagesSent == 1);

    // If we drop receipts we should keep trying to send messages every time we
    // "deliverOfflineMsgs"
    fixture.friendMessageSender.dropReceipts = true;
    fixture.friendMessageSender.sendMessage(0, msg.content, receipt);
    fixture.offlineMsgEngine.addSentMessage(receipt, msg, completionFn);
    fixture.offlineMsgEngine.deliverOfflineMsgs();
    fixture.offlineMsgEngine.deliverOfflineMsgs();
    fixture.offlineMsgEngine.deliverOfflineMsgs();

    QVERIFY(fixture.friendMessageSender.numMessagesSent == 5);

    // And once we stop dropping and try one more time we should run out of
    // messages to send again
    fixture.friendMessageSender.dropReceipts = false;
    fixture.offlineMsgEngine.deliverOfflineMsgs();
    fixture.offlineMsgEngine.deliverOfflineMsgs();
    fixture.offlineMsgEngine.deliverOfflineMsgs();
    QVERIFY(fixture.friendMessageSender.numMessagesSent == 6);
}

void TestOfflineMsgEngine::testOfflineFriend()
{
    OfflineMsgEngineFixture fixture;

    Message msg{false, QString(), QDateTime()};

    fixture.f.setStatus(Status::Status::Offline);

    fixture.offlineMsgEngine.addUnsentMessage(msg, completionFn);
    fixture.offlineMsgEngine.addUnsentMessage(msg, completionFn);
    fixture.offlineMsgEngine.addUnsentMessage(msg, completionFn);
    fixture.offlineMsgEngine.addUnsentMessage(msg, completionFn);
    fixture.offlineMsgEngine.addUnsentMessage(msg, completionFn);

    fixture.f.setStatus(Status::Status::Online);
    fixture.offlineMsgEngine.deliverOfflineMsgs();


    QVERIFY(fixture.friendMessageSender.numMessagesFailed == 0);
    QVERIFY(fixture.friendMessageSender.numMessagesSent == 5);
}

void TestOfflineMsgEngine::testSentUnsentCoordination()
{
    OfflineMsgEngineFixture fixture;
    Message msg{false, QString("a"), QDateTime()};
    ReceiptNum receipt;

    fixture.offlineMsgEngine.addUnsentMessage(msg, completionFn);
    msg.content = "b";
    fixture.friendMessageSender.dropReceipts = true;
    fixture.friendMessageSender.sendMessage(0, msg.content, receipt);
    fixture.friendMessageSender.dropReceipts = false;
    fixture.offlineMsgEngine.addSentMessage(receipt, msg, completionFn);
    msg.content = "c";
    fixture.offlineMsgEngine.addUnsentMessage(msg, completionFn);

    fixture.offlineMsgEngine.deliverOfflineMsgs();

    auto expectedResponseOrder = std::vector<QString>{"a", "b", "c"};
    QVERIFY(fixture.friendMessageSender.msgs == expectedResponseOrder);
}

void TestOfflineMsgEngine::testCallback()
{
    OfflineMsgEngineFixture fixture;

    size_t numCallbacks = 0;
    auto callback = [&numCallbacks] { numCallbacks++; };
    Message msg{false, QString(), QDateTime()};
    ReceiptNum receipt;

    fixture.friendMessageSender.sendMessage(0, msg.content, receipt);
    fixture.offlineMsgEngine.addSentMessage(receipt, msg, callback);
    fixture.offlineMsgEngine.addUnsentMessage(msg, callback);

    fixture.offlineMsgEngine.deliverOfflineMsgs();
    QVERIFY(numCallbacks == 2);
}

QTEST_GUILESS_MAIN(TestOfflineMsgEngine)
#include "offlinemsgengine_test.moc"
