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
#include "src/persistence/settings.h"

GroupMessageDispatcher::GroupMessageDispatcher(Group* g_, MessageProcessor processor_)
    : group(g_)
    , processor(processor_)
{
    processor.enableMentions();
}

std::pair<DispatchedMessageId, DispatchedMessageId>
GroupMessageDispatcher::sendMessage(bool isAction, QString const& content)
{
    const auto firstMessageId = nextMessageId;
    auto lastMessageId = firstMessageId;

    for (auto const& message : processor.processOutgoingMessage(isAction, content)) {
        auto messageId = nextMessageId++;
        lastMessageId = messageId;
        if (group->getPeersCount() != 1) {
            if (message.isAction) {
                Core::getInstance()->sendGroupAction(group->getId(), message.content);
            } else {
                Core::getInstance()->sendGroupMessage(group->getId(), message.content);
            }
        }

        // Emit both signals since we do not have receipts for groups
        //
        // NOTE: We could in theory keep track of our sent message and wait for
        // toxcore to send it back to us to indicate a completed message, but
        // this isn't necessarily the design of toxcore and associating the
        // received message back would be difficult.
        emit this->messageSent(messageId, message);
        emit this->messageComplete(messageId);
    }

    return std::make_pair(firstMessageId, lastMessageId);
}

/**
 * @brief Processes and dispatches received message from toxcore
 * @param[in] sender
 * @param[in] isAction True if is action
 * @param[in] content Message content
 */
void GroupMessageDispatcher::onMessageReceived(const ToxPk& sender, bool isAction, QString const& content)
{
    auto core = Core::getInstance();
    auto const& settings = Settings::getInstance();
    bool isSelf = sender == core->getSelfId().getPublicKey();

    if (isSelf) {
        return;
    }

    if (settings.getBlackList().contains(sender.toString())) {
        qDebug() << "onGroupMessageReceived: Filtered:" << sender.toString();
        return;
    }

    emit messageReceived(sender, processor.processIncomingMessage(isAction, content));
}
