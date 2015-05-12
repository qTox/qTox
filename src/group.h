/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
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
struct ToxID;

class Group : public QObject
{
    Q_OBJECT
public:
    Group(int GroupId, QString Name, bool IsAvGroupchat);
    virtual ~Group();

    bool isAvGroupchat() const;
    int getGroupId() const;
    int getPeersCount() const;
    void regeneratePeerList();
    QStringList getPeerList() const;
    bool isSelfPeerNumber(int peernumber) const;

    GroupChatForm *getChatForm();
    GroupWidget *getGroupWidget();

    void setEventFlag(int f);
    int getEventFlag() const;

    void setMentionedFlag(int f);
    int getMentionedFlag() const;

    /*
    void addPeer(int peerId, QString name);
    void removePeer(int peerId);
    */

    void updatePeer(int peerId, QString newName);
    void setName(const QString& name);

    QString resolveToxID(const ToxID &id) const;

private:
    GroupWidget* widget;
    GroupChatForm* chatForm;
    QStringList peers;
    QMap<QString, QString> toxids;
    int hasNewMessages, userWasMentioned;
    int groupId;
    int nPeers;
    int selfPeerNum = -1;
    bool avGroupchat;

};

#endif // GROUP_H
