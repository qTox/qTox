/*
    Copyright © 2019 by The qTox Project Contributors

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

#include "contentdialogmanager.h"

#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/widget/friendwidget.h"
#include "src/widget/groupwidget.h"

#include <tuple>

namespace {
void removeDialog(ContentDialog* dialog, QHash<const ChatId&, ContentDialog*>& dialogs)
{
    for (auto it = dialogs.begin(); it != dialogs.end();) {
        if (*it == dialog) {
            it = dialogs.erase(it);
        } else {
            ++it;
        }
    }
}
} // namespace

ContentDialogManager::ContentDialogManager(FriendList& friendList_)
    : friendList{friendList_}
{
}

ContentDialog* ContentDialogManager::current()
{
    return currentDialog;
}

bool ContentDialogManager::chatWidgetExists(const ChatId& chatId)
{
    const auto dialog = chatDialogs.value(chatId, nullptr);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->hasChat(chatId);
}

FriendWidget* ContentDialogManager::addFriendToDialog(ContentDialog* dialog,
                                                      std::shared_ptr<FriendChatroom> chatroom,
                                                      GenericChatForm* form)
{
    auto friendWidget = dialog->addFriend(chatroom, form);
    const auto& friendPk = friendWidget->getFriend()->getPublicKey();

    ContentDialog* lastDialog = getFriendDialog(friendPk);
    if (lastDialog) {
        lastDialog->removeFriend(friendPk);
    }

    chatDialogs[friendPk] = dialog;
    return friendWidget;
}

GroupWidget* ContentDialogManager::addGroupToDialog(ContentDialog* dialog,
                                                    std::shared_ptr<GroupChatroom> chatroom,
                                                    GenericChatForm* form)
{
    auto groupWidget = dialog->addGroup(chatroom, form);
    const auto& groupId = groupWidget->getGroup()->getPersistentId();

    ContentDialog* lastDialog = getGroupDialog(groupId);
    if (lastDialog) {
        lastDialog->removeGroup(groupId);
    }

    chatDialogs[groupId] = dialog;
    return groupWidget;
}

void ContentDialogManager::focusChat(const ChatId& chatId)
{
    auto dialog = focusDialog(chatId, chatDialogs);
    if (dialog != nullptr) {
        dialog->focusChat(chatId);
    }
}

/**
 * @brief Focus the dialog if it exists.
 * @param id User Id.
 * @param list List with dialogs
 * @return ContentDialog if found, nullptr otherwise
 */
ContentDialog* ContentDialogManager::focusDialog(const ChatId& id,
                                                 const QHash<const ChatId&, ContentDialog*>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return nullptr;
    }

    ContentDialog* dialog = *iter;
    if (dialog->windowState() & Qt::WindowMinimized) {
        dialog->showNormal();
    }

    dialog->raise();
    dialog->activateWindow();
    return dialog;
}

void ContentDialogManager::updateFriendStatus(const ToxPk& friendPk)
{
    auto dialog = chatDialogs.value(friendPk);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateChatStatusLight(friendPk);
    if (dialog->isChatActive(friendPk)) {
        dialog->updateTitleAndStatusIcon();
    }

    Friend* f = friendList.findFriend(friendPk);
    dialog->updateFriendStatus(friendPk, f->getStatus());
}

void ContentDialogManager::updateGroupStatus(const GroupId& groupId)
{
    auto dialog = chatDialogs.value(groupId);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateChatStatusLight(groupId);
    if (dialog->isChatActive(groupId)) {
        dialog->updateTitleAndStatusIcon();
    }
}

bool ContentDialogManager::isChatActive(const ChatId& chatId)
{
    const auto dialog = chatDialogs.value(chatId);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->isChatActive(chatId);
}

ContentDialog* ContentDialogManager::getFriendDialog(const ToxPk& friendPk) const
{
    return chatDialogs.value(friendPk);
}

ContentDialog* ContentDialogManager::getGroupDialog(const GroupId& groupId) const
{
    return chatDialogs.value(groupId);
}

void ContentDialogManager::addContentDialog(ContentDialog& dialog)
{
    currentDialog = &dialog;
    connect(&dialog, &ContentDialog::willClose, this, &ContentDialogManager::onDialogClose);
    connect(&dialog, &ContentDialog::activated, this, &ContentDialogManager::onDialogActivate);
}

void ContentDialogManager::onDialogActivate()
{
    ContentDialog* dialog = qobject_cast<ContentDialog*>(sender());
    currentDialog = dialog;
}

void ContentDialogManager::onDialogClose()
{
    ContentDialog* dialog = qobject_cast<ContentDialog*>(sender());
    if (currentDialog == dialog) {
        currentDialog = nullptr;
    }

    removeDialog(dialog, chatDialogs);
}

IDialogs* ContentDialogManager::getFriendDialogs(const ToxPk& friendPk) const
{
    return getFriendDialog(friendPk);
}

IDialogs* ContentDialogManager::getGroupDialogs(const GroupId& groupId) const
{
    return getGroupDialog(groupId);
}
