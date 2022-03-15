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

class TestOfflineMsgEngine : public QObject
{
    Q_OBJECT

private slots:
    void testReceiptBeforeMessage();
    void testReceiptAfterMessage();
    void testResendWorkflow();
    void testTypeCoordination();
    void testCallback();
    void testExtendedMessageCoordination();
};

namespace {
void completionFn(bool success) { std::ignore = success; }
} // namespace

void TestOfflineMsgEngine::testReceiptBeforeMessage()
{
    OfflineMsgEngine offlineMsgEngine;

    Message msg{false, QString(), QDateTime(), {}, {}};

    auto const receipt = ReceiptNum(0);
    offlineMsgEngine.onReceiptReceived(receipt);
    offlineMsgEngine.addSentCoreMessage(receipt, Message(), completionFn);

    auto const removedMessages = offlineMsgEngine.removeAllMessages();

    QVERIFY(removedMessages.empty());
}

void TestOfflineMsgEngine::testReceiptAfterMessage()
{
    OfflineMsgEngine offlineMsgEngine;

    auto const receipt = ReceiptNum(0);
    offlineMsgEngine.addSentCoreMessage(receipt, Message(), completionFn);
    offlineMsgEngine.onReceiptReceived(receipt);

    auto const removedMessages = offlineMsgEngine.removeAllMessages();

    QVERIFY(removedMessages.empty());
}

void TestOfflineMsgEngine::testResendWorkflow()
{
    OfflineMsgEngine offlineMsgEngine;

    auto const receipt = ReceiptNum(0);
    offlineMsgEngine.addSentCoreMessage(receipt, Message(), completionFn);
    auto messagesToResend = offlineMsgEngine.removeAllMessages();

    QVERIFY(messagesToResend.size() == 1);

    offlineMsgEngine.addSentCoreMessage(receipt, Message(), completionFn);
    offlineMsgEngine.onReceiptReceived(receipt);

    messagesToResend = offlineMsgEngine.removeAllMessages();
    QVERIFY(messagesToResend.size() == 0);

    auto const nullMsg = Message();
    auto msg2 = Message();
    auto msg3 = Message();
    msg2.content = "msg2";
    msg3.content = "msg3";
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(0), nullMsg, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(1), nullMsg, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(2), msg2, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(3), msg3, completionFn);

    offlineMsgEngine.onReceiptReceived(ReceiptNum(0));
    offlineMsgEngine.onReceiptReceived(ReceiptNum(1));
    offlineMsgEngine.onReceiptReceived(ReceiptNum(3));

    messagesToResend = offlineMsgEngine.removeAllMessages();
    QVERIFY(messagesToResend.size() == 1);
    QVERIFY(messagesToResend[0].message.content == "msg2");
}


void TestOfflineMsgEngine::testTypeCoordination()
{
    OfflineMsgEngine offlineMsgEngine;

    auto msg1 = Message();
    auto msg2 = Message();
    auto msg3 = Message();
    auto msg4 = Message();
    auto msg5 = Message();

    msg1.content = "msg1";
    msg2.content = "msg2";
    msg3.content = "msg3";
    msg4.content = "msg4";
    msg5.content = "msg5";

    offlineMsgEngine.addSentCoreMessage(ReceiptNum(1), msg1, completionFn);
    offlineMsgEngine.addUnsentMessage(msg2, completionFn);
    offlineMsgEngine.addSentExtendedMessage(ExtendedReceiptNum(1), msg3, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(2), msg4, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(3), msg5, completionFn);

    const auto messagesToResend = offlineMsgEngine.removeAllMessages();

    QVERIFY(messagesToResend[0].message.content == "msg1");
    QVERIFY(messagesToResend[1].message.content == "msg2");
    QVERIFY(messagesToResend[2].message.content == "msg3");
    QVERIFY(messagesToResend[3].message.content == "msg4");
    QVERIFY(messagesToResend[4].message.content == "msg5");
}

void TestOfflineMsgEngine::testCallback()
{
    OfflineMsgEngine offlineMsgEngine;

    size_t numCallbacks = 0;
    auto callback = [&numCallbacks] (bool) { numCallbacks++; };
    Message msg{false, QString(), QDateTime(), {}, {}};
    ReceiptNum receipt;

    offlineMsgEngine.addSentCoreMessage(ReceiptNum(1), Message(), callback);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(2), Message(), callback);

    offlineMsgEngine.onReceiptReceived(ReceiptNum(1));
    offlineMsgEngine.onReceiptReceived(ReceiptNum(2));

    QVERIFY(numCallbacks == 2);
}

void TestOfflineMsgEngine::testExtendedMessageCoordination()
{
    OfflineMsgEngine offlineMsgEngine;

    size_t numCallbacks = 0;
    auto callback = [&numCallbacks] (bool) { numCallbacks++; };

    auto msg1 = Message();
    auto msg2 = Message();
    auto msg3 = Message();

    offlineMsgEngine.addSentCoreMessage(ReceiptNum(1), msg1, callback);
    offlineMsgEngine.addSentExtendedMessage(ExtendedReceiptNum(1), msg1, callback);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(2), msg1, callback);

    offlineMsgEngine.onExtendedReceiptReceived(ExtendedReceiptNum(2));
    QVERIFY(numCallbacks == 0);

    offlineMsgEngine.onReceiptReceived(ReceiptNum(1));
    QVERIFY(numCallbacks == 1);

    offlineMsgEngine.onReceiptReceived(ReceiptNum(1));
    QVERIFY(numCallbacks == 1);

    offlineMsgEngine.onExtendedReceiptReceived(ExtendedReceiptNum(1));
    QVERIFY(numCallbacks == 2);

    offlineMsgEngine.onReceiptReceived(ReceiptNum(2));
    QVERIFY(numCallbacks == 3);
}

QTEST_GUILESS_MAIN(TestOfflineMsgEngine)
#include "offlinemsgengine_test.moc"
