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

class Core;
class IDialogsManager;
class Group;
class ToxPk;
class FriendList;

class GroupChatroom : public QObject, public Chatroom
{
    Q_OBJECT
public:
    GroupChatroom(Group* group_, IDialogsManager* dialogsManager_, Core& core_,
        FriendList& friendList);

    Chat* getChat() override;

    Group* getGroup();

    bool hasNewMessage() const;
    void resetEventFlags();

    bool friendExists(const ToxPk& pk);
    void inviteFriend(const ToxPk& pk);

    bool possibleToOpenInNewWindow() const;
    bool canBeRemovedFromWindow() const;
    void removeGroupFromDialogs();

private:
    Group* group{nullptr};
    IDialogsManager* dialogsManager{nullptr};
    Core& core;
    FriendList& friendList;
};
