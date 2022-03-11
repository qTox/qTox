/*
    Copyright © 2020 by The qTox Project Contributors

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

#include "notificationgenerator.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "util/display.h"

#include <QCollator>

namespace
{
    size_t getNumMessages(
        const QHash<const Friend*, size_t>& friendNotifications,
        const QHash<const Group* , size_t>& groupNotifications)
    {
        auto numMessages = std::accumulate(friendNotifications.begin(), friendNotifications.end(), 0);
        numMessages = std::accumulate(groupNotifications.begin(), groupNotifications.end(), numMessages);

        return numMessages;
    }

    size_t getNumChats(
        const QHash<const Friend*, size_t>& friendNotifications,
        const QHash<const Group* , size_t>& groupNotifications)
    {
        return friendNotifications.size() + groupNotifications.size();
    }

    QString generateMultiChatTitle(size_t numChats, size_t numMessages)
    {
        //: e.g. 3 messages from 2 chats
        return QObject::tr("%1 message(s) from %2 chats")
            .arg(numMessages)
            .arg(numChats);
    }

    template <typename T>
    QString generateSingleChatTitle(
        const QHash<T, size_t> numNotifications,
        T contact)
    {
        if (numNotifications[contact] > 1)
        {
            //: e.g. 2 messages from Bob
            return QObject::tr("%1 message(s) from %2")
                .arg(numNotifications[contact])
                .arg(contact->getDisplayedName());
        }
        else
        {
            return contact->getDisplayedName();
        }
    }

    QString generateTitle(
        const QHash<const Friend*, size_t>& friendNotifications,
        const QHash<const Group* , size_t>& groupNotifications,
        const Friend* f)
    {
        auto numChats = getNumChats(friendNotifications, groupNotifications);
        if (numChats > 1)
        {
            return generateMultiChatTitle(numChats, getNumMessages(friendNotifications, groupNotifications));
        }
        else
        {
            return generateSingleChatTitle(friendNotifications, f);
        }
    }

    QString generateTitle(
        const QHash<const Friend*, size_t>& friendNotifications,
        const QHash<const Group* , size_t>& groupNotifications,
        const Group* g)
    {
        auto numChats = getNumChats(friendNotifications, groupNotifications);
        if (numChats > 1)
        {
            return generateMultiChatTitle(numChats, getNumMessages(friendNotifications, groupNotifications));
        }
        else
        {
            return generateSingleChatTitle(groupNotifications, g);
        }
    }

    QString generateContent(
        const QHash<const Friend*, size_t>& friendNotifications,
        const QHash<const Group*, size_t>& groupNotifications,
        QString lastMessage,
        const ToxPk& sender)
    {
        assert(friendNotifications.size() > 0 || groupNotifications.size() > 0);

        auto numChats = getNumChats(friendNotifications, groupNotifications);
        if (numChats > 1) {
            // Copy all names into a vector to simplify formatting logic between
            // multiple lists
            std::vector<QString> displayNames;
            displayNames.reserve(numChats);

            for (auto it = friendNotifications.begin(); it != friendNotifications.end(); ++it) {
                displayNames.push_back(it.key()->getDisplayedName());
            }

            for (auto it = groupNotifications.begin(); it != groupNotifications.end(); ++it) {
                displayNames.push_back(it.key()->getDisplayedName());
            }

            assert(displayNames.size() > 0);

            // Lexiographically sort all display names to ensure consistent formatting
            QCollator collator;
            std::sort(displayNames.begin(), displayNames.end(), [&] (const QString& a, const QString& b) {
                return collator.compare(a, b) < 1;
            });

            auto it = displayNames.begin();

            QString ret = *it;

            while (++it != displayNames.end()) {
                ret += ", " + *it;
            }

            return ret;
        }
        else {
            if (groupNotifications.size() == 1) {
                return groupNotifications.begin().key()->getPeerList()[sender] + ": " + lastMessage;
            }

            return lastMessage;
        }
    }

    QPixmap getSenderAvatar(Profile* profile, const ToxPk& sender)
    {
        return profile ? profile->loadAvatar(sender) : QPixmap();
    }
} // namespace

NotificationGenerator::NotificationGenerator(
    INotificationSettings const& notificationSettings_,
    Profile* profile_)
    : notificationSettings(notificationSettings_)
    , profile(profile_)
{}

NotificationGenerator::~NotificationGenerator() = default;

NotificationData NotificationGenerator::friendMessageNotification(const Friend* f, const QString& message)
{
    friendNotifications[f]++;

    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("New message");
        return ret;
    }

    ret.title = generateTitle(friendNotifications, groupNotifications, f);
    ret.message = generateContent(friendNotifications, groupNotifications, message, f->getPublicKey());
    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::groupMessageNotification(const Group* g, const ToxPk& sender, const QString& message)
{
    groupNotifications[g]++;

    NotificationData ret;

    if (notificationSettings.getNotifyHide()){
        ret.title = tr("New group message");
        return ret;
    }

    ret.title = generateTitle(friendNotifications, groupNotifications, g);
    ret.message = generateContent(friendNotifications, groupNotifications, message, sender);
    ret.pixmap = getSenderAvatar(profile, sender);

    return ret;
}

NotificationData NotificationGenerator::fileTransferNotification(const Friend* f, const QString& filename, size_t fileSize)
{
    friendNotifications[f]++;

    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Incoming file transfer");
        return ret;
    }

    auto numChats = getNumChats(friendNotifications, groupNotifications);
    auto numMessages = getNumMessages(friendNotifications, groupNotifications);

    if (numChats > 1 || numMessages > 1)
    {
        ret.title = generateTitle(friendNotifications, groupNotifications, f);
        ret.message = generateContent(friendNotifications, groupNotifications, tr("Incoming file transfer"), f->getPublicKey());
    }
    else
    {
        //: e.g. Bob - file transfer
        ret.title = tr("%1 - file transfer").arg(f->getDisplayedName());
        ret.message = filename + " (" + getHumanReadableSize(fileSize) + ")";
    }

    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::groupInvitationNotification(const Friend* from)
{
    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Group invite received");
        return ret;
    }

    ret.title = tr("%1 invites you to join a group.").arg(from->getDisplayedName());
    ret.message = "";
    ret.pixmap = getSenderAvatar(profile, from->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::friendRequestNotification(const ToxPk& sender, const QString& message)
{
    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Friend request received");
        return ret;
    }

    ret.title = tr("Friend request received from %1").arg(sender.toString());
    ret.message = message;

    return ret;
}

void NotificationGenerator::onNotificationActivated()
{
    friendNotifications = {};
    groupNotifications = {};
}
