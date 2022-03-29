/*
    Copyright © 2018-2019 by The qTox Project Contributors

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

#include "contentdialog.h"
#include "src/core/chatid.h"
#include "src/core/groupid.h"
#include "src/core/toxpk.h"
#include "src/model/dialogs/idialogsmanager.h"

#include <QObject>

/**
 * @breaf Manage all content dialogs
 */
class ContentDialogManager : public QObject, public IDialogsManager
{
    Q_OBJECT
public:
    explicit ContentDialogManager(FriendList& friendList);
    ContentDialog* current();
    bool chatWidgetExists(const ChatId& chatId);
    void focusChat(const ChatId& chatId);
    void updateFriendStatus(const ToxPk& friendPk);
    void updateGroupStatus(const GroupId& groupId);
    bool isChatActive(const ChatId& chatId);
    ContentDialog* getFriendDialog(const ToxPk& friendPk) const;
    ContentDialog* getGroupDialog(const GroupId& groupId) const;

    IDialogs* getFriendDialogs(const ToxPk& friendPk) const;
    IDialogs* getGroupDialogs(const GroupId& groupId) const;

    FriendWidget* addFriendToDialog(ContentDialog* dialog, std::shared_ptr<FriendChatroom> chatroom,
                                    GenericChatForm* form);
    GroupWidget* addGroupToDialog(ContentDialog* dialog, std::shared_ptr<GroupChatroom> chatroom,
                                  GenericChatForm* form);

    void addContentDialog(ContentDialog& dialog);

private slots:
    void onDialogClose();
    void onDialogActivate();

private:
    ContentDialog* focusDialog(const ChatId& id,
                               const QHash<const ChatId&, ContentDialog*>& list);

    ContentDialog* currentDialog = nullptr;

    QHash<const ChatId&, ContentDialog*> chatDialogs;
    FriendList& friendList;
};
