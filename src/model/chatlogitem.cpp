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

#include "chatlogitem.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/friend.h"
#include "src/model/group.h"

#include <cassert>

namespace {

/**
 * Helper template to get the correct deleter function for our type erased unique_ptr
 */
template <typename T>
struct ChatLogItemDeleter
{
    static void doDelete(void* ptr)
    {
        delete static_cast<T*>(ptr);
    }
};

QString resolveToxPk(const ToxPk& pk)
{
    Friend* f = FriendList::findFriend(pk);
    if (f) {
        return f->getDisplayedName();
    }

    for (Group* it : GroupList::getAllGroups()) {
        QString res = it->resolveToxId(pk);
        if (!res.isEmpty()) {
            return res;
        }
    }

    return pk.toString();
}

QString resolveSenderNameFromSender(const ToxPk& sender)
{
    // TODO: Remove core instance
    const Core* core = Core::getInstance();

    // In unit tests we don't have a core instance so we just stringize the key
    if (!core) {
        return sender.toString();
    }

    bool isSelf = sender == core->getSelfId().getPublicKey();
    QString myNickName = core->getUsername().isEmpty() ? sender.toString() : core->getUsername();

    return isSelf ? myNickName : resolveToxPk(sender);
}
} // namespace

ChatLogItem::ChatLogItem(ToxPk sender_, ChatLogFile file_)
    : ChatLogItem(std::move(sender_), ContentType::fileTransfer,
                  ContentPtr(new ChatLogFile(std::move(file_)),
                             ChatLogItemDeleter<ChatLogFile>::doDelete))
{}

ChatLogItem::ChatLogItem(ToxPk sender_, ChatLogMessage message_)
    : ChatLogItem(sender_, ContentType::message,
                  ContentPtr(new ChatLogMessage(std::move(message_)),
                             ChatLogItemDeleter<ChatLogMessage>::doDelete))
{}

ChatLogItem::ChatLogItem(ToxPk sender_, ContentType contentType_, ContentPtr content_)
    : sender(std::move(sender_))
    , displayName(resolveSenderNameFromSender(sender))
    , contentType(contentType_)
    , content(std::move(content_))
{}

const ToxPk& ChatLogItem::getSender() const
{
    return sender;
}

ChatLogItem::ContentType ChatLogItem::getContentType() const
{
    return contentType;
}

ChatLogFile& ChatLogItem::getContentAsFile()
{
    assert(contentType == ContentType::fileTransfer);
    return *static_cast<ChatLogFile*>(content.get());
}

const ChatLogFile& ChatLogItem::getContentAsFile() const
{
    assert(contentType == ContentType::fileTransfer);
    return *static_cast<ChatLogFile*>(content.get());
}

ChatLogMessage& ChatLogItem::getContentAsMessage()
{
    assert(contentType == ContentType::message);
    return *static_cast<ChatLogMessage*>(content.get());
}

const ChatLogMessage& ChatLogItem::getContentAsMessage() const
{
    assert(contentType == ContentType::message);
    return *static_cast<ChatLogMessage*>(content.get());
}

QDateTime ChatLogItem::getTimestamp() const
{
    switch (contentType) {
    case ChatLogItem::ContentType::message: {
        const auto& message = getContentAsMessage();
        return message.message.timestamp;
    }
    case ChatLogItem::ContentType::fileTransfer: {
        const auto& file = getContentAsFile();
        return file.timestamp;
    }
    }

    assert(false);
    return QDateTime();
}

void ChatLogItem::setDisplayName(QString name)
{
    displayName = name;
}

const QString& ChatLogItem::getDisplayName() const
{
    return displayName;
}
