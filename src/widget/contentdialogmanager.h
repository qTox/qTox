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
    void updateGroupStatus(int groupId);
    bool isFriendWidgetActive(int friendId);
    bool isGroupWidgetActive(int groupId);
    ContentDialog* getFriendDialog(int friendId) const;
    ContentDialog* getGroupDialog(int groupId) const;

    FriendWidget* addFriendToDialog(ContentDialog* dialog, std::shared_ptr<FriendChatroom> chatroom, GenericChatForm* form);
    GroupWidget* addGroupToDialog(ContentDialog* dialog, std::shared_ptr<GroupChatroom> chatroom, GenericChatForm* form);

    void addContentDialog(ContentDialog* dialog);

    static ContentDialogManager* getInstance();

private slots:
    void onDialogClose();
    void onDialogActivate();

private:
    ContentDialog* focusDialog(int id, const QHash<int, ContentDialog*>& list);

    ContentDialog* currentDialog = nullptr;

    QHash<int, ContentDialog*> friendDialogs;
    QHash<int, ContentDialog*> groupDialogs;

    static ContentDialogManager* instance;
};

#endif // _CONTENT_DIALOG_MANAGER_H_
