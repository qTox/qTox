/*
    Copyright © 2018 by The qTox Project Contributors

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

#ifndef _CONTENT_DIALOG_MANAGER_H_
#define _CONTENT_DIALOG_MANAGER_H_

#include <QObject>
#include "contentdialog.h"

/**
 * @breaf Manage all content dialogs
 */
class ContentDialogManager : public QObject
{
    Q_OBJECT
public:
    ContentDialog* current();
    bool friendWidgetExists(int friendId);
    bool groupWidgetExists(int groupId);
    void focusFriend(int friendId);
    void focusGroup(int groupId);
    void updateFriendStatus(int friendId);
    void updateFriendStatusMessage(int friendId, const QString& message);
    void updateGroupStatus(int groupId);
    bool isFriendWidgetActive(int friendId);
    bool isGroupWidgetActive(int groupId);
    ContentDialog* getFriendDialog(int friendId) const;
    ContentDialog* getGroupDialog(int groupId) const;

    void removeFriend(int friendId);
    void removeGroup(int groupId);

    FriendWidget* addFriendToDialog(ContentDialog* dialog, std::shared_ptr<FriendChatroom> chatroom, GenericChatForm* form);
    GroupWidget* addGroupToDialog(ContentDialog* dialog, std::shared_ptr<GroupChatroom> chatroom, GenericChatForm* form);

    void addContentDialog(ContentDialog* dialog);

    static ContentDialogManager* getInstance();

    bool hasFriendWidget(ContentDialog* dialog, int friendId, const GenericChatroomWidget* chatroomWidget) const;
    bool hasGroupWidget(ContentDialog* dialog, int groupId, const GenericChatroomWidget* chatroomWidget) const;

    FriendWidget* getFriendWidget(int friendId) const;

private slots:
    void onDialogClose();
    void onDialogActivate();

private:
    bool existsWidget(int id, const QHash<int, ContactInfo>& list);
    void focusDialog(int id, const QHash<int, ContactInfo>& list);
    void updateStatus(int id, const QHash<int, ContactInfo>& list);
    bool isWidgetActive(int id, const QHash<int, ContactInfo>& list);
    ContentDialog* getDialog(int id, const QHash<int, ContactInfo>& list) const;

    ContentDialog* currentDialog = nullptr;
    QHash<int, ContactInfo> friendList;
    QHash<int, ContactInfo> groupList;

    static ContentDialogManager* instance;
};

#endif // _CONTENT_DIALOG_MANAGER_H_
