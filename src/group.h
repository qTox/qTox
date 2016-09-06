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

#ifndef GROUP_H
#define GROUP_H

#include <QMap>
#include <QObject>
#include <QStringList>

#define RETRY_PEER_INFO_INTERVAL 500

class Friend;
class GroupWidget;
class GroupChatForm;
class ToxId;

class Group : public QObject
{
    Q_OBJECT
public:
    static Group* add(int groupId, const QString &name, bool isAvGroupchat);
    static Group* get(int groupId);
    static QList<Group *> getAll();
    static void remove(int groupId);
    static void removeAll();

public:
    bool isAvGroupchat() const;
    int getGroupId() const;
    int getPeersCount() const;
    void regeneratePeerList();
    QStringList getPeerList() const;
    bool isSelfPeerNumber(int peernumber) const;

    GroupChatForm *getChatForm();
    GroupWidget *getGroupWidget();

    void setEventFlag(bool f);
    bool getEventFlag() const;

    void setMentionedFlag(bool f);
    bool getMentionedFlag() const;

    void updatePeer(int peerId, QString newName);
    void setName(const QString& name);
    QString getName() const;

    QString resolveToxId(const ToxId &id) const;

signals:
    void titleChanged(GroupWidget* widget);
    void userListChanged(GroupWidget* widget);

private:
    class Private;
    Group::Private* data;
    static QHash<int, Group::Private*> groupList;

private:
    explicit Group(Group::Private* data);
};

#endif // GROUP_H
