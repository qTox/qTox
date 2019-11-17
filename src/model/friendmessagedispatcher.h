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

#ifndef FRIEND_MESSAGE_DISPATCHER_H
#define FRIEND_MESSAGE_DISPATCHER_H

#include "src/core/icorefriendmessagesender.h"
#include "src/model/friend.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/message.h"
#include "src/persistence/offlinemsgengine.h"

#include <QObject>
#include <QString>

#include <cstdint>

class FriendMessageDispatcher : public IMessageDispatcher
{
    Q_OBJECT
public:
    FriendMessageDispatcher(Friend& f, MessageProcessor processor,
                            ICoreFriendMessageSender& messageSender,
                            ICoreExtPacketAllocator& coreExt);

    std::pair<DispatchedMessageId, DispatchedMessageId> sendMessage(bool isAction,
                                                                    const QString& content) override;

    DispatchedMessageId sendExtendedMessage(const QString& content, ExtensionSet extensions) override;
    void onMessageReceived(bool isAction, const QString& content);
    void onReceiptReceived(ReceiptNum receipt);
    void onExtMessageReceived(const QString& message);
    void onExtReceiptReceived(uint64_t receiptId);
    void clearOutgoingMessages();
private slots:
    void onFriendStatusChange(const ToxPk& key, Status::Status status);

private:
    void sendProcessedMessage(Message const& msg, OfflineMsgEngine::CompletionFn fn);
    void sendExtendedProcessedMessage(Message const& msg, OfflineMsgEngine::CompletionFn fn);
    void sendCoreProcessedMessage(Message const& msg, OfflineMsgEngine::CompletionFn fn);
    OfflineMsgEngine::CompletionFn getCompletionFn(DispatchedMessageId messageId);

    Friend& f;
    ICoreExtPacketAllocator& coreExtPacketAllocator;
    DispatchedMessageId nextMessageId = DispatchedMessageId(0);

    ICoreFriendMessageSender& messageSender;
    OfflineMsgEngine offlineMsgEngine;
    MessageProcessor processor;
};


#endif /* IMESSAGE_DISPATCHER_H */
