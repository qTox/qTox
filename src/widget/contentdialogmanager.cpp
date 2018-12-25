#include "contentdialogmanager.h"

#include "src/widget/friendwidget.h"
#include "src/widget/groupwidget.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/friend.h"
#include "src/model/group.h"

#include <tuple>

namespace
{
void removeDialog(ContentDialog* dialog, QHash<int, ContentDialog*>& dialogs)
{
    for (auto it = dialogs.begin(); it != dialogs.end();) {
        if (*it == dialog) {
            it = dialogs.erase(it);
        } else {
            ++it;
        }
    }
}
}

ContentDialogManager* ContentDialogManager::instance;

ContentDialog* ContentDialogManager::current()
{
    return currentDialog;
}

bool ContentDialogManager::friendWidgetExists(int friendId)
{
    const auto dialog = friendDialogs.value(friendId, nullptr);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->containsFriend(friendId);
}

bool ContentDialogManager::groupWidgetExists(int groupId)
{
    const auto dialog = groupDialogs.value(groupId, nullptr);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->containsGroup(groupId);
}

FriendWidget* ContentDialogManager::addFriendToDialog(ContentDialog* dialog,
    std::shared_ptr<FriendChatroom> chatroom, GenericChatForm* form)
{
    auto friendWidget = dialog->addFriend(chatroom, form);
    auto friendId = friendWidget->getFriend()->getId();

    ContentDialog* lastDialog = getFriendDialog(friendId);
    if (lastDialog) {
        lastDialog->removeFriend(friendId);
    }

    friendDialogs[friendId] = dialog;
    return friendWidget;
}

GroupWidget* ContentDialogManager::addGroupToDialog(ContentDialog* dialog,
    std::shared_ptr<GroupChatroom> chatroom, GenericChatForm* form)
{
    auto groupWidget = dialog->addGroup(chatroom, form);
    auto groupId = groupWidget->getGroup()->getId();

    ContentDialog* lastDialog = getGroupDialog(groupId);
    if (lastDialog) {
        lastDialog->removeGroup(groupId);
    }

    groupDialogs[groupId] = dialog;
    return groupWidget;
}

void ContentDialogManager::focusFriend(int friendId)
{
    auto dialog = focusDialog(friendId, friendDialogs);
    if (dialog != nullptr) {
        dialog->focusFriend(friendId);
    }
}

void ContentDialogManager::focusGroup(int groupId)
{
    auto dialog = focusDialog(groupId, groupDialogs);
    if (dialog != nullptr) {
        dialog->focusGroup(groupId);
    }
}

/**
 * @brief Focus the dialog if it exists.
 * @param id User Id.
 * @param list List with dialogs
 * @return ContentDialog if found, nullptr otherwise
 */
ContentDialog* ContentDialogManager::focusDialog(int id, const QHash<int, ContentDialog*>& list)
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

void ContentDialogManager::updateFriendStatus(int friendId)
{
    auto dialog = friendDialogs.value(friendId);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateFriendStatusLight(friendId);
    if (dialog->isFriendWidgetActive(friendId)) {
        dialog->updateTitleAndStatusIcon();
    }

    Friend* f = FriendList::findFriend(friendId);
    dialog->updateFriendStatus(friendId, f->getStatus());
}

void ContentDialogManager::updateGroupStatus(int groupId)
{
    auto dialog = friendDialogs.value(groupId);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateGroupStatusLight(groupId);
    if (dialog->isGroupWidgetActive(groupId)) {
        dialog->updateTitleAndStatusIcon();
    }
}

bool ContentDialogManager::isFriendWidgetActive(int friendId)
{
    const auto dialog = friendDialogs.value(friendId);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->isFriendWidgetActive(friendId);
}

bool ContentDialogManager::isGroupWidgetActive(int groupId)
{
    const auto dialog = groupDialogs.value(groupId);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->isGroupWidgetActive(groupId);
}

ContentDialog* ContentDialogManager::getFriendDialog(int friendId) const
{
    return friendDialogs.value(friendId);
}

ContentDialog* ContentDialogManager::getGroupDialog(int groupId) const
{
    return groupDialogs.value(groupId);
}

ContentDialogManager* ContentDialogManager::getInstance()
{
    if (instance == nullptr) {
        instance = new ContentDialogManager();
    }

    return instance;
}

void ContentDialogManager::addContentDialog(ContentDialog* dialog)
{
    currentDialog = dialog;
    connect(dialog, &ContentDialog::willClose, this, &ContentDialogManager::onDialogClose);
    connect(dialog, &ContentDialog::activated, this, &ContentDialogManager::onDialogActivate);
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

    removeDialog(dialog, friendDialogs);
    removeDialog(dialog, groupDialogs);
}
