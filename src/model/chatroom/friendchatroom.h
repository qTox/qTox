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

#ifndef FRIEND_CHATROOM_H
#define FRIEND_CHATROOM_H

#include <QObject>
#include <QString>

class Friend;
class Group;

class FriendChatroom : public QObject
{
    Q_OBJECT
public:
    FriendChatroom(Friend* frnd);

public slots:
    Friend* getFriend();

    void setActive(bool active);

    bool canBeInvited() const;

    int getCircleId() const;
    QString getCircleName() const;

    void inviteToNewGroup();
    void inviteFriend(uint32_t friendId, const Group* group);

    bool autoAcceptEnabled() const;
    QString getAutoAcceptDir() const;
    void disableAutoAccept();
    void setAutoAcceptDir(const QString& dir);

signals:
    void activeChanged(bool activated);

private:
    bool active{false};
    Friend* frnd{nullptr};
};

#endif // FRIEND_H
