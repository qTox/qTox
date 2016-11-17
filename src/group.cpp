/*
    Copyright © 2014-2016 by The qTox Project

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

#include "src/core/core.h"
#include "src/friend.h"
#include "src/grouplist.h"
#include "src/widget/form/groupchatform.h"
#include "src/widget/gui.h"
#include "src/widget/groupwidget.h"
#include <QDebug>
#include <QTimer>

GroupNotify Group::notifier;

Group* Group::get(int groupId)
{
    return GroupList::findGroup(groupId);
}

Group::List Group::getAll()
{
    return GroupList::getAllGroups();
}

void Group::remove(int groupId)
{
    GroupList::removeGroup(groupId);
}

Group::Group(Group::ID groupId, QString name, bool isAvGroupchat)
    : groupId(groupId)
    , title(name)
    , nPeers{0}
    , avGroupchat{isAvGroupchat}
{
}

Group::~Group()
{
}

void Group::updatePeer(int peerId, QString name)
{
    ToxId id = Core::getInstance()->getGroupPeerToxId(groupId, peerId);
    QString toxid = id.publicKey;
    peers[peerId] = name;
    toxids[toxid] = name;

    Friend f = Friend::get(id);
    if (f && f.hasAlias())
    {
        peers[peerId] = f.getDisplayedName();
        toxids[toxid] = f.getDisplayedName();
    }
    else
    {
        emit notifier.userListChanged(*this, nPeers, 0);
    }
}

void Group::setName(const QString& name)
{
    if (title != name)
    {
        title = name;
        emit notifier.titleChanged(*this, title);
    }
}

QString Group::getName() const
{
    return title;
}

void Group::regeneratePeerList()
{
    const Core* core = Core::getInstance();
    peers = core->getGroupPeerNames(groupId);
    toxids.clear();
    nPeers = peers.size();
    for (int i = 0; i < nPeers; ++i)
    {
        ToxId id = core->getGroupPeerToxId(groupId, i);
        ToxId self = core->getSelfId();
        if (id == self)
            selfPeerNum = i;

        QString toxid = id.publicKey;
        toxids[toxid] = peers[i];
        if (toxids[toxid].isEmpty())
            toxids[toxid] = QObject::tr("<Empty>", "Placeholder when someone's name in a group chat is empty");

        Friend f = Friend::get(id);
        if (f && f.hasAlias())
        {
            peers[i] = f.getDisplayedName();
            toxids[toxid] = f.getDisplayedName();
        }
    }

    emit notifier.userListChanged(*this, nPeers, 0);
}

bool Group::isAvGroupchat() const
{
    return avGroupchat;
}

Group::ID Group::getGroupId() const
{
    return groupId;
}

int Group::getPeersCount() const
{
    return nPeers;
}

QStringList Group::getPeerList() const
{
    return peers;
}

bool Group::isSelfPeerNumber(int num) const
{
    return num == selfPeerNum;
}

QString Group::resolveToxId(const ToxId &id) const
{
    QString key = id.publicKey;
    auto it = toxids.find(key);

    if (it != toxids.end())
        return *it;

    return QString();
}

/**
 * @brief constructor
 *
 * Registers the @a Group class to the Qt meta object system.
 */
GroupNotify::GroupNotify()
{
    // TODO:
    //qRegisterMetaType<Group>("Group");
}
