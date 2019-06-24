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

#include "src/model/message.h"

#include <tox/tox.h>

#include <QObject>
#include <QtTest/QtTest>

namespace {
bool messageHasSelfMention(const Message& message)
{
    return std::any_of(message.metadata.begin(), message.metadata.end(), [](MessageMetadata meta) {
        return meta.type == MessageMetadataType::selfMention;
    });
}
} // namespace

class TestMessageProcessor : public QObject
{
    Q_OBJECT

public:
    TestMessageProcessor(){};

private slots:
    void testSelfMention();
    void testOutgoingMessage();
    void testIncomingMessage();
};


/**
 * @brief Tests detection of username
 */
void TestMessageProcessor::testSelfMention()
{
    MessageProcessor::SharedParams sharedParams;
    sharedParams.onUserNameSet("MyUserName");

    auto messageProcessor = MessageProcessor(sharedParams);
    messageProcessor.enableMentions();

    // Using my name should match
    auto processedMessage = messageProcessor.processIncomingMessage(false, "MyUserName hi");
    QVERIFY(messageHasSelfMention(processedMessage));

    // Action messages should match too
    processedMessage = messageProcessor.processIncomingMessage(true, "MyUserName hi");
    QVERIFY(messageHasSelfMention(processedMessage));

    // Too much text shouldn't match
    processedMessage = messageProcessor.processIncomingMessage(false, "MyUserName2");
    QVERIFY(!messageHasSelfMention(processedMessage));

    // Unless it's a colon
    processedMessage = messageProcessor.processIncomingMessage(false, "MyUserName: test");
    QVERIFY(messageHasSelfMention(processedMessage));

    // Too little text shouldn't match
    processedMessage = messageProcessor.processIncomingMessage(false, "MyUser");
    QVERIFY(!messageHasSelfMention(processedMessage));

    // The regex should be case insensitive
    processedMessage = messageProcessor.processIncomingMessage(false, "myusername hi");
    QVERIFY(messageHasSelfMention(processedMessage));

    // New user name changes should be detected
    sharedParams.onUserNameSet("NewUserName");
    processedMessage = messageProcessor.processIncomingMessage(false, "NewUserName: hi");
    QVERIFY(messageHasSelfMention(processedMessage));

    // Special characters should be removed
    sharedParams.onUserNameSet("New\nUserName");
    processedMessage = messageProcessor.processIncomingMessage(false, "NewUserName: hi");
    QVERIFY(messageHasSelfMention(processedMessage));
}

/**
 * @brief Tests behavior of the processor for outgoing messages
 */
void TestMessageProcessor::testOutgoingMessage()
{
    auto sharedParams = MessageProcessor::SharedParams();
    auto messageProcessor = MessageProcessor(sharedParams);

    QString testStr;

    for (size_t i = 0; i < tox_max_message_length() + 50; ++i) {
        testStr += "a";
    }

    auto messages = messageProcessor.processOutgoingMessage(false, testStr);

    // The message processor should split our messages
    QVERIFY(messages.size() == 2);
}

/**
 * @brief Tests behavior of the processor for incoming messages
 */
void TestMessageProcessor::testIncomingMessage()
{
    // Nothing too special happening on the incoming side if we aren't looking for self mentions
    auto sharedParams = MessageProcessor::SharedParams();
    auto messageProcessor = MessageProcessor(sharedParams);
    auto message = messageProcessor.processIncomingMessage(false, "test");

    QVERIFY(message.isAction == false);
    QVERIFY(message.content == "test");
    QVERIFY(message.timestamp.isValid());
}

QTEST_GUILESS_MAIN(TestMessageProcessor)
#include "messageprocessor_test.moc"
