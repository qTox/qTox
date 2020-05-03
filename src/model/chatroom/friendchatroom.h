/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#pragma once

#include "chatroom.h"

#include <QObject>
#include <QString>
#include <QVector>

class IDialogsManager;
class Friend;
class Group;

struct GroupToDisplay
{
    QString name;
    Group* group;
};

struct CircleToDisplay
{
    QString name;
    int circleId;
};

class FriendChatroom : public QObject, public Chatroom
{
    Q_OBJECT
public:
    FriendChatroom(Friend* frnd, IDialogsManager* dialogsManager);

    Contact* getContact() override;

public slots:

    Friend* getFriend();

    void setActive(bool active);

    bool canBeInvited() const;

    int getCircleId() const;
    QString getCircleName() const;

    void inviteToNewGroup();
    void inviteFriend(const Group* group);

    bool autoAcceptEnabled() const;
    QString getAutoAcceptDir() const;
    void disableAutoAccept();
    void setAutoAcceptDir(const QString& dir);

    QVector<GroupToDisplay> getGroups() const;
    QVector<CircleToDisplay> getOtherCircles() const;

    void resetEventFlags();

    bool possibleToOpenInNewWindow() const;
    bool canBeRemovedFromWindow() const;
    bool friendCanBeRemoved() const;
    void removeFriendFromDialogs();

signals:
    void activeChanged(bool activated);

private:
    bool active{false};
    Friend* frnd{nullptr};
    IDialogsManager* dialogsManager{nullptr};
};
