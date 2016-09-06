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
#include "grouplist.h"
#include "src/core/core.h"
#include "widget/gui.h"
#include <QDebug>
#include <QTimer>

class Group::Private
{
public:
    Private(int groupId, const QString& name, bool isAvGroupchat)
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

QHash<uint32_t, Group::Private*> Group::groupList;

Group::Group(int groupId, const QString& name, bool isAvGroupchat)
{
    auto checker = groupList.find(groupId);
    if (checker != groupList.end())
        qWarning() << "addGroup: groupId already taken";

    data = new Group::Private(groupId, name, isAvGroupchat);
    groupList[groupId] = data;
}

Group::~Group()
{
    auto g_it = groupList.find(data->groupId);
    if (g_it == groupList.end())
        return;

    delete *g_it;
    groupList.erase(g_it);
}

Group* Group::get(int groupId)
{
    auto g_it = groupList.find(groupId);
    if (g_it == groupList.end())
        return nullptr;

    return new Group(*g_it);
}

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

Group::Group(Group::Private* data)
    : data(data)
{}

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

void Group::setName(const QString& name)
{
    data->chatForm->setName(name);

    if (data->widget->isActive())
        GUI::setWindowTitle(name);

    emit titleChanged(data->widget);
}

QString Group::getName() const
{
    return data->widget->getName();
}

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
        if (f != nullptr && f->hasAlias())
        {
            data->peers[i] = f->getDisplayedName();
            data->toxids[toxid] = f->getDisplayedName();
        }
    }

    data->widget->onUserListChanged();
    data->chatForm->onUserListChanged();
    emit userListChanged(getGroupWidget());
}

bool Group::isAvGroupchat() const
{
    return data->avGroupchat;
}

int Group::getGroupId() const
{
    return data->groupId;
}

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

QStringList Group::getPeerList() const
{
    return data->peers;
}

bool Group::isSelfPeerNumber(int num) const
{
    return num == data->selfPeerNum;
}

void Group::setEventFlag(bool f)
{
    data->hasNewMessages = f;
}

bool Group::getEventFlag() const
{
    return data->hasNewMessages;
}

void Group::setMentionedFlag(bool f)
{
    data->userWasMentioned = f;
}

bool Group::getMentionedFlag() const
{
    return data->userWasMentioned;
}

QString Group::resolveToxId(const ToxId &id) const
{
    QString key = id.publicKey;
    auto it = data->toxids.find(key);

    if (it != data->toxids.end())
        return *it;

    return QString();
}
