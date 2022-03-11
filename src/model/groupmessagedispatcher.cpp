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

#include "groupmessagedispatcher.h"
#include "src/persistence/igroupsettings.h"

#include <QtCore>

GroupMessageDispatcher::GroupMessageDispatcher(Group& g_, MessageProcessor processor_,
                                               ICoreIdHandler& idHandler_,
                                               ICoreGroupMessageSender& messageSender_,
                                               const IGroupSettings& groupSettings_)
    : group(g_)
    , processor(processor_)
    , idHandler(idHandler_)
    , messageSender(messageSender_)
    , groupSettings(groupSettings_)
{
    processor.enableMentions();
}

std::pair<DispatchedMessageId, DispatchedMessageId>
GroupMessageDispatcher::sendMessage(bool isAction, QString const& content)
{
    const auto firstMessageId = nextMessageId;
    auto lastMessageId = firstMessageId;

    for (auto const& message : processor.processOutgoingMessage(isAction, content, ExtensionSet())) {
        auto messageId = nextMessageId++;
        lastMessageId = messageId;
        if (group.getPeersCount() != 1) {
            if (message.isAction) {
                messageSender.sendGroupAction(group.getId(), message.content);
            } else {
                messageSender.sendGroupMessage(group.getId(), message.content);
            }
        }

        // Emit both signals since we do not have receipts for groups
        //
        // NOTE: We could in theory keep track of our sent message and wait for
        // toxcore to send it back to us to indicate a completed message, but
        // this isn't necessarily the design of toxcore and associating the
        // received message back would be difficult.
        emit messageSent(messageId, message);
        emit messageComplete(messageId);
    }

    return std::make_pair(firstMessageId, lastMessageId);
}

std::pair<DispatchedMessageId, DispatchedMessageId>
GroupMessageDispatcher::sendExtendedMessage(const QString& content, ExtensionSet extensions)
{
    std::ignore = extensions;
    // Stub this api to immediately fail
    auto messageId = nextMessageId++;
    auto messages = processor.processOutgoingMessage(false, content, ExtensionSet());
    emit messageSent(messageId, messages[0]);
    emit messageBroken(messageId, BrokenMessageReason::unsupportedExtensions);
    return {messageId, messageId};
}

/**
 * @brief Processes and dispatches received message from toxcore
 * @param[in] sender
 * @param[in] isAction True if is action
 * @param[in] content Message content
 */
void GroupMessageDispatcher::onMessageReceived(const ToxPk& sender, bool isAction, QString const& content)
{
    bool isSelf = sender == idHandler.getSelfPublicKey();

    if (isSelf) {
        return;
    }

    if (groupSettings.getBlackList().contains(sender.toString())) {
        qDebug() << "onGroupMessageReceived: Filtered:" << sender.toString();
        return;
    }

    emit messageReceived(sender, processor.processIncomingCoreMessage(isAction, content));
}
