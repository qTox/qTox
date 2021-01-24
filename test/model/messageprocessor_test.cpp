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
#include <QStringBuilder>
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
    TestMessageProcessor(){}

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
    MessageProcessor::SharedParams sharedParams(tox_max_message_length(), 10 * 1024 * 1024);;
    const QLatin1String testUserName{"MyUserName"};
    const QLatin1String testToxPk{
        "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"};
    sharedParams.onUserNameSet(testUserName);
    sharedParams.setPublicKey(testToxPk);

    auto messageProcessor = MessageProcessor(sharedParams);
    messageProcessor.enableMentions();

    for (const auto& str : {testUserName, testToxPk}) {

        // Using my name or public key should match
        auto processedMessage = messageProcessor.processIncomingCoreMessage(false, str % " hi");
        QVERIFY(messageHasSelfMention(processedMessage));

        // Action messages should match too
        processedMessage = messageProcessor.processIncomingCoreMessage(true, str % " hi");
        QVERIFY(messageHasSelfMention(processedMessage));

        // Too much text shouldn't match
        processedMessage = messageProcessor.processIncomingCoreMessage(false, str % "2");
        QVERIFY(!messageHasSelfMention(processedMessage));

        // Unless it's a colon
        processedMessage = messageProcessor.processIncomingCoreMessage(false, str % ": test");
        QVERIFY(messageHasSelfMention(processedMessage));

        // remove last character
        QString chopped = str;
        chopped.chop(1);
        // Too little text shouldn't match
        processedMessage = messageProcessor.processIncomingCoreMessage(false, chopped);
        QVERIFY(!messageHasSelfMention(processedMessage));

        // make lower case
        QString lower = QString(str).toLower();

        // The regex should be case insensitive
        processedMessage = messageProcessor.processIncomingCoreMessage(false, lower % " hi");
        QVERIFY(messageHasSelfMention(processedMessage));
    }

    // New user name changes should be detected
    sharedParams.onUserNameSet("NewUserName");
    auto processedMessage = messageProcessor.processIncomingCoreMessage(false, "NewUserName: hi");
    QVERIFY(messageHasSelfMention(processedMessage));

    // Special characters should be removed
    sharedParams.onUserNameSet("New\nUserName");
    processedMessage = messageProcessor.processIncomingCoreMessage(false, "NewUserName: hi");
    QVERIFY(messageHasSelfMention(processedMessage));

    // Regression tests for: https://github.com/qTox/qTox/issues/2119
    {
        // Empty usernames should not match
        sharedParams.onUserNameSet("");
        processedMessage = messageProcessor.processIncomingCoreMessage(false, "");
        QVERIFY(!messageHasSelfMention(processedMessage));

        // Empty usernames matched on everything, ensure this is not the case
        processedMessage = messageProcessor.processIncomingCoreMessage(false, "a");
        QVERIFY(!messageHasSelfMention(processedMessage));
    }
}

/**
 * @brief Tests behavior of the processor for outgoing messages
 */
void TestMessageProcessor::testOutgoingMessage()
{
    auto sharedParams = MessageProcessor::SharedParams(tox_max_message_length(), 10 * 1024 * 1024);
    auto messageProcessor = MessageProcessor(sharedParams);

    QString testStr;

    for (size_t i = 0; i < tox_max_message_length() + 50; ++i) {
        testStr += "a";
    }

    auto messages = messageProcessor.processOutgoingMessage(false, testStr, ExtensionSet());

    // The message processor should split our messages
    QVERIFY(messages.size() == 2);

    auto extensionSet = ExtensionSet();
    extensionSet[ExtensionType::messages] = true;
    messages = messageProcessor.processOutgoingMessage(false, testStr, extensionSet);

    // If we have multipart messages we shouldn't split our messages
    QVERIFY(messages.size() == 1);
}

/**
 * @brief Tests behavior of the processor for incoming messages
 */
void TestMessageProcessor::testIncomingMessage()
{
    // Nothing too special happening on the incoming side if we aren't looking for self mentions
    auto sharedParams = MessageProcessor::SharedParams(tox_max_message_length(), 10 * 1024 * 1024);
    auto messageProcessor = MessageProcessor(sharedParams);
    auto message = messageProcessor.processIncomingCoreMessage(false, "test");

    QVERIFY(message.isAction == false);
    QVERIFY(message.content == "test");
    QVERIFY(message.timestamp.isValid());
}

QTEST_GUILESS_MAIN(TestMessageProcessor)
#include "messageprocessor_test.moc"
