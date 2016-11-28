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
class GroupNotify;
class GroupWidget;
class GroupChatForm;
class ToxId;

class Group final
{
    class Private;

public:
    using ID = int;
    using Groups = QHash<ID, Private*>;
    using List = QList<Group*>;

private:
    static GroupNotify notifier;

public:
    static Group* get(int groupId);
    static List getAll();
    static void remove(int groupId);

    inline static const GroupNotify* notify()
    {
        return &notifier;
    }

    inline static QString statusToString(const Group* g)
    {
        Q_UNUSED(g);
        return QObject::tr("Online");
    }

public:
    Group(ID groupId, QString name, bool isAvGroupchat);
    ~Group();

    bool isAvGroupchat() const;
    ID getGroupId() const;
    int getPeersCount() const;
    void regeneratePeerList();
    QStringList getPeerList() const;
    bool isSelfPeerNumber(int peernumber) const;

    void setMentionedFlag(bool f);
    bool getMentionedFlag() const;

    void updatePeer(int peerId, QString newName);
    void setName(const QString& name);
    QString getName() const;

    QString resolveToxId(const ToxId &id) const;

private:
    ID groupId;
    QString title;
    QStringList peers;
    QMap<QString, QString> toxids;
    int nPeers;
    int selfPeerNum = -1;
    bool avGroupchat;
};

class GroupNotify : public QObject
{
    Q_OBJECT
public:
    GroupNotify();
signals:
    void titleChanged(const Group& g, QString title);
    void userListChanged(const Group& g, int numPeers, quint8 change);
};

#endif // GROUP_H
