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
#include "src/core/contactid.h"
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
    ContentDialog* current();
    bool contactWidgetExists(const ContactId& groupId);
    void focusContact(const ContactId& contactId);
    void updateFriendStatus(const ToxPk& friendPk);
    void updateGroupStatus(const GroupId& friendPk);
    bool isContactActive(const ContactId& contactId);
    ContentDialog* getFriendDialog(const ToxPk& friendPk) const;
    ContentDialog* getGroupDialog(const GroupId& friendPk) const;

    IDialogs* getFriendDialogs(const ToxPk& friendPk) const;
    IDialogs* getGroupDialogs(const GroupId& groupId) const;

    FriendWidget* addFriendToDialog(ContentDialog* dialog, std::shared_ptr<FriendChatroom> chatroom,
                                    GenericChatForm* form);
    GroupWidget* addGroupToDialog(ContentDialog* dialog, std::shared_ptr<GroupChatroom> chatroom,
                                  GenericChatForm* form);

    void addContentDialog(ContentDialog& dialog);

    static ContentDialogManager* getInstance();

private slots:
    void onDialogClose();
    void onDialogActivate();

private:
    ContentDialog* focusDialog(const ContactId& id,
                               const QHash<const ContactId&, ContentDialog*>& list);

    ContentDialog* currentDialog = nullptr;

    QHash<const ContactId&, ContentDialog*> contactDialogs;

    static ContentDialogManager* instance;
};
