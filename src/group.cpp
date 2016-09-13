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

#include "widget/groupwidget.h"
#include "widget/form/groupchatform.h"
#include "friend.h"
#include "src/core/core.h"
#include "widget/gui.h"
#include <QDebug>
#include <QTimer>

class Group::Private
{
public:
    Private(Group::ID groupId, const QString& name, bool isAvGroupchat)
        : groupId(groupId)
        , nPeers(0)
        , avGroupchat(isAvGroupchat)

    {
        widget = new GroupWidget(groupId, name);
        chatForm = new GroupChatForm(new Group(this));

        //in groupchats, we only notify on messages containing your name <-- dumb
        // sound notifications should be on all messages, but system popup notification
        // on naming is appropriate
        hasNewMessages = 0;
        userWasMentioned = 0;
    }

    ~Private()
    {
        delete chatForm;
        widget->deleteLater();
    }

    GroupWidget* widget;
    GroupChatForm* chatForm;
    QStringList peers;
    QMap<QString, QString> toxids;
    bool hasNewMessages;
    bool userWasMentioned;
    int groupId;
    int nPeers;
    int selfPeerNum = -1;
    bool avGroupchat;
};

QHash<Group::ID, Group::Private*> Group::groupList;

/**
 * @brief Group constructor.
 * @param groupId The group's ID.
 * @param name Name of the group.
 * @param isAvGroupchat True, if is AV groupchat, false, otherwise.
 *
 * Add new group in the group list.
 */
Group::Group(Group::ID groupId, const QString& name, bool isAvGroupchat)
{
    auto checker = groupList.find(groupId);
    if (checker != groupList.end())
        qWarning() << "addGroup: groupId already taken";

    data = new Group::Private(groupId, name, isAvGroupchat);
    groupList[groupId] = data;
}

/**
 * @brief Group constructor, without adding group to db.
 * @param data Private group data.
 */
Group::Group(Group::Private* data)
    : data(data)
{
}

/**
 * @brief Group destructor.
 *
 * Removes a group from the group list.
 */
Group::~Group()
{
    auto g_it = groupList.find(data->groupId);
    if (g_it == groupList.end())
        return;

    delete *g_it;
    groupList.erase(g_it);
}

/**
 * @brief Looks up a group in the friend list.
 * @param groupId The lookup ID.
 * @return The group if found; nullptr otherwise.
 */
Group* Group::get(Group::ID groupId)
{
    auto g_it = groupList.find(groupId);
    if (g_it == groupList.end())
        return nullptr;

    return new Group(*g_it);
}

/**
 * @brief Get list of all existing groups.
 * @return List of all existing groups.
 */
QList<Group*> Group::getAll()
{
    QList<Group*> res;

    for (Private* it : groupList)
    {
        Group *group = new Group(it);
        res.append(group);
    }

    return res;
}

/**
 * @brief Update friend name in the group.
 * @param peerId Peer ID.
 * @param name Peer name.
 *
 * Set new name if peer already exist, add peer otherwise.
 */
void Group::updatePeer(int peerId, QString name)
{
    ToxId id = Core::getInstance()->getGroupPeerToxId(data->groupId, peerId);
    QString toxid = id.publicKey;
    data->peers[peerId] = name;
    data->toxids[toxid] = name;

    Friend *f = Friend::get(id);
    if (f != nullptr && f->hasAlias())
    {
        data->peers[peerId] = f->getDisplayedName();
        data->toxids[toxid] = f->getDisplayedName();
    }
    else
    {
        data->widget->onUserListChanged();
        data->chatForm->onUserListChanged();
        emit userListChanged(getGroupWidget());
    }
}

/**
 * @brief Set displayed name of group.
 * @param name New group name.
 */
void Group::setName(const QString& name)
{
    data->chatForm->setName(name);

    if (data->widget->isActive())
        GUI::setWindowTitle(name);

    emit titleChanged(data->widget);
}

/**
 * @brief Return name, which should be displayed.
 * @return Group displayed name.
 */
QString Group::getName() const
{
    return data->widget->getName();
}

/**
 * @brief Regenerate list of peers in the group.
 */
void Group::regeneratePeerList()
{
    data->peers = Core::getInstance()->getGroupPeerNames(data->groupId);
    data->toxids.clear();
    data->nPeers = data->peers.size();
    for (int i = 0; i < data->nPeers; i++)
    {
        ToxId id = Core::getInstance()->getGroupPeerToxId(data->groupId, i);
        if (id.isSelf())
            data->selfPeerNum = i;

        QString toxid = id.publicKey;
        data->toxids[toxid] = data->peers[i];
        if (data->toxids[toxid].isEmpty())
            data->toxids[toxid] = tr("<Empty>", "Placeholder when someone's name in a group chat is empty");

        Friend *f = Friend::get(id);
        if (f && f->hasAlias())
        {
            data->peers[i] = f->getDisplayedName();
            data->toxids[toxid] = f->getDisplayedName();
        }
    }

    data->widget->onUserListChanged();
    data->chatForm->onUserListChanged();
    emit userListChanged(getGroupWidget());
}

/**
 * @brief Get information about audio/video in the group.
 * @return True if group support audio/video, false otherwise.
 */
bool Group::isAvGroupchat() const
{
    return data->avGroupchat;
}

/**
 * @brief Get group ID.
 * @return Group ID.
 */
int Group::getGroupId() const
{
    return data->groupId;
}

/**
 * @brief Get peers count.
 * @return Count of peers in groupchat.
 */
int Group::getPeersCount() const
{
    return data->nPeers;
}

GroupChatForm *Group::getChatForm()
{
    return data->chatForm;
}

GroupWidget *Group::getGroupWidget()
{
    return data->widget;
}

/**
 * @brief Get peer list
 * @return List of peer's names in group.
 */
QStringList Group::getPeerList() const
{
    return data->peers;
}

/**
 * @brief Check, that self user have this number in group.
 * @param num Peer ID.
 * @return True if self user stored in group by "num" ID.
 */
bool Group::isSelfPeerNumber(int num) const
{
    return num == data->selfPeerNum;
}

/**
 * @brief Set event flag
 * @param flag True if group has new event, false otherwise.
 */
void Group::setEventFlag(bool flag)
{
    data->hasNewMessages = flag;
}

/**
 * @brief Get event flag.
 * @return Return true, if group has new event, false otherwise.
 */
bool Group::getEventFlag() const
{
    return data->hasNewMessages;
}

/**
 * @brief Set mentioned flag
 * @param flag True if someone mentione user in group, false otherwise.
 */
void Group::setMentionedFlag(bool f)
{
    data->userWasMentioned = f;
}

/**
 * @brief Get mentioned flag.
 * @return Return true, if someone mentione user in group, false otherwise.
 */
bool Group::getMentionedFlag() const
{
    return data->userWasMentioned;
}

/**
 * @brief Get name of peer with a certain id.
 * @param id Tox ID of peer.
 * @return Name of peer.
 */
QString Group::resolveToxId(const ToxId &id) const
{
    QString key = id.publicKey;
    auto it = data->toxids.find(key);

    if (it != data->toxids.end())
        return *it;

    return QString();
}
