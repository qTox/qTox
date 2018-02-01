/*
    Copyright © 2014-2017 by The qTox Project Contributors

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

#include "contact.h"
#include <QMap>
#include <QObject>
#include <QStringList>

#define RETRY_PEER_INFO_INTERVAL 500

class Friend;
class GroupChatForm;
class ToxPk;

class Group : public Contact
{
    Q_OBJECT
public:
    Group(int groupId, const QString& name, bool isAvGroupchat, const QString& selfName);

    bool isAvGroupchat() const;
    uint32_t getId() const override;
    int getPeersCount() const;
    void regeneratePeerList();
    QStringList getPeerList() const;
    bool isSelfPeerNumber(int peernumber) const;

    void setEventFlag(bool f) override;
    bool getEventFlag() const override;

    void setMentionedFlag(bool f);
    bool getMentionedFlag() const;

    void updatePeer(int peerId, QString newName);
    void setName(const QString& newTitle) override;
    void onTitleChanged(const QString& author, const QString& newTitle);
    QString getName() const;
    QString getDisplayedName() const override;

    QString resolveToxId(const ToxPk& id) const;
    void setSelfName(const QString& name);

signals:
    void titleChangedByUser(uint32_t groupId, const QString& title);
    void titleChanged(uint32_t groupId, const QString& author, const QString& title);
    void userListChanged(uint32_t groupId, const QMap<QByteArray, QString>& toxids);

private:
    QString selfName;
    QString title;
    GroupChatForm* chatForm;
    QStringList peers;
    QMap<QByteArray, QString> toxids;
    bool hasNewMessages;
    bool userWasMentioned;
    int groupId;
    int nPeers;
    int selfPeerNum = -1;
    bool avGroupchat;
};

#endif // GROUP_H
