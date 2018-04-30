/*
    Copyright © 2014-2018 by The qTox Project Contributors

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

#include "group.h"
#include "friend.h"
#include "src/friendlist.h"
#include "src/core/core.h"
#include "src/persistence/settings.h"
#include "src/widget/form/groupchatform.h"
#include "src/widget/groupwidget.h"
#include <QDebug>

static const int MAX_GROUP_TITLE_LENGTH = 128;

Group::Group(int groupId, const QString& name, bool isAvGroupchat, const QString& selfName)
    : selfName{selfName}
    , title{name}
    , groupId(groupId)
    , nPeers{0}
    , avGroupchat{isAvGroupchat}
{
    // in groupchats, we only notify on messages containing your name <-- dumb
    // sound notifications should be on all messages, but system popup notification
    // on naming is appropriate
    hasNewMessages = 0;
    userWasMentioned = 0;
    regeneratePeerList();
}

void Group::updatePeer(int peerId, QString name)
{
    ToxPk peerKey = Core::getInstance()->getGroupPeerPk(groupId, peerId);
    QByteArray peerPk = peerKey.getKey();
    toxids[peerPk] = name;

    Friend* f = FriendList::findFriend(peerKey);
    if (f != nullptr) {
        // use the displayed name from the friends list
        toxids[peerPk] = f->getDisplayedName();
    } else {
        emit userListChanged(groupId, toxids);
    }
}

void Group::setName(const QString& newTitle)
{
    const QString shortTitle = newTitle.left(MAX_GROUP_TITLE_LENGTH);
    if (!shortTitle.isEmpty() && title != shortTitle) {
        title = shortTitle;
        emit displayedNameChanged(title);
        emit titleChangedByUser(groupId, title);
        emit titleChanged(groupId, selfName, title);
    }
}

void Group::setTitle(const QString& author, const QString& newTitle)
{
    const QString shortTitle = newTitle.left(MAX_GROUP_TITLE_LENGTH);
    if (!shortTitle.isEmpty() && title != shortTitle) {
        title = shortTitle;
        emit displayedNameChanged(title);
        emit titleChanged(groupId, author, title);
    }
}

QString Group::getName() const
{
    return title;
}

QString Group::getDisplayedName() const
{
    return getName();
}

void Group::regeneratePeerList()
{
    const Core* core = Core::getInstance();
    const ToxPk self = core->getSelfId().getPublicKey();

    QStringList peers = core->getGroupPeerNames(groupId);
    toxids.clear();
    nPeers = peers.size();
    for (int i = 0; i < nPeers; ++i) {
        ToxPk id = core->getGroupPeerPk(groupId, i);
        if (id == self) {
            selfPeerNum = i;
        }

        QByteArray peerPk = id.getKey();
        toxids[peerPk] = peers[i];
        if (toxids[peerPk].isEmpty()) {
            toxids[peerPk] =
                tr("<Empty>", "Placeholder when someone's name in a group chat is empty");
        }

        Friend* f = FriendList::findFriend(id);
        if (f != nullptr && f->hasAlias()) {
            toxids[peerPk] = f->getDisplayedName();
        }
    }

    emit userListChanged(groupId, toxids);
}

bool Group::isAvGroupchat() const
{
    return avGroupchat;
}

uint32_t Group::getId() const
{
    return groupId;
}

int Group::getPeersCount() const
{
    return nPeers;
}

QStringList Group::getPeerList() const
{
    return toxids.values();
}

bool Group::isSelfPeerNumber(int num) const
{
    return num == selfPeerNum;
}

void Group::setEventFlag(bool f)
{
    hasNewMessages = f;
}

bool Group::getEventFlag() const
{
    return hasNewMessages;
}

void Group::setMentionedFlag(bool f)
{
    userWasMentioned = f;
}

bool Group::getMentionedFlag() const
{
    return userWasMentioned;
}

QString Group::resolveToxId(const ToxPk& id) const
{
    QByteArray key = id.getKey();
    auto it = toxids.find(key);

    if (it != toxids.end()) {
        return *it;
    }

    return QString();
}

void Group::setSelfName(const QString& name)
{
    selfName = name;
}
