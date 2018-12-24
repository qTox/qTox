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
void removeDialog(ContentDialog* dialog, QHash<int, ContactInfo>& infos)
{
    for (auto it = infos.begin(); it != infos.end();) {
        if (std::get<0>(*it) == dialog) {
            it = infos.erase(it);
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
    return existsWidget(friendId, friendList);
}

bool ContentDialogManager::groupWidgetExists(int groupId)
{
    return existsWidget(groupId, groupList);
}

FriendWidget* ContentDialogManager::addFriendToDialog(ContentDialog* dialog, 
    std::shared_ptr<FriendChatroom> chatroom, GenericChatForm* form)
{
    auto friendWidget = dialog->addFriend(chatroom, form);
    auto friendId = friendWidget->getFriend()->getId();

    ContentDialog* lastDialog = getFriendDialog(friendId);
    if (lastDialog) {
        lastDialog->removeFriend(friendWidget);
    }

    friendList.insert(friendId, std::make_tuple(dialog, friendWidget));
    return friendWidget;
}

GroupWidget* ContentDialogManager::addGroupToDialog(ContentDialog* dialog,
    std::shared_ptr<GroupChatroom> chatroom, GenericChatForm* form)
{
    auto groupWidget = dialog->addGroup(chatroom, form);
    auto groupId = groupWidget->getGroup()->getId();

    ContentDialog* lastDialog = getGroupDialog(groupId);
    if (lastDialog) {
        lastDialog->removeGroup(groupWidget);
    }

    groupList.insert(groupId, std::make_tuple(dialog, groupWidget));
    return groupWidget;
}

// TODO: Remove method. Move logic in ContentDialog
void ContentDialogManager::removeFriend(int friendId)
{
    auto iter = friendList.find(friendId);
    if (iter == friendList.end()) {
        return;
    }

    auto friendWidget = static_cast<FriendWidget*>(std::get<1>(iter.value()));
    auto dialog = getFriendDialog(friendId);
    dialog->removeFriend(friendWidget);
    friendList.remove(friendId);
}

void ContentDialogManager::removeGroup(int groupId)
{
    auto iter = friendList.find(groupId);
    if (iter == friendList.end()) {
        return;
    }

    auto groupWidget = static_cast<GroupWidget*>(std::get<1>(iter.value()));
    auto dialog = getGroupDialog(groupId);
    dialog->removeGroup(groupWidget);
    groupList.remove(groupId);
}

/**
 * @brief Check, if widget is exists.
 * @param id User Id.
 * @param list List with contact info.
 * @return True is widget exists, false otherwise.
 */
bool ContentDialogManager::existsWidget(int id, const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);
    return iter != list.end();
}

void ContentDialogManager::focusFriend(int friendId)
{
    focusDialog(friendId, friendList);
}

void ContentDialogManager::focusGroup(int groupId)
{
    focusDialog(groupId, groupList);
}

/**
 * @brief Focus the dialog if it exists.
 * @param id User Id.
 * @param list List with contact info.
 */
void ContentDialogManager::focusDialog(int id, const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return;
    }

    ContentDialog* dialog = std::get<0>(*iter);
    if (dialog->windowState() & Qt::WindowMinimized) {
        dialog->showNormal();
    }

    dialog->raise();
    dialog->activateWindow();
    dialog->activate(std::get<1>(iter.value()));
}

void ContentDialogManager::updateFriendStatus(int friendId)
{
    updateStatus(friendId, friendList);
    ContentDialog* contentDialog = getFriendDialog(friendId);
    if (contentDialog) {
        auto iter = friendList.find(friendId).value();
        GenericChatroomWidget* widget = std::get<1>(iter);
        FriendWidget* friendWidget = static_cast<FriendWidget*>(widget);

        Friend* f = FriendList::findFriend(friendId);
        contentDialog->addFriendWidget(friendWidget, f->getStatus());
    }
}

/**
 * @brief Update friend status message.
 * @param friendId Id friend, whose status was changed.
 * @param message Status message.
 */
void ContentDialogManager::updateFriendStatusMessage(int friendId, const QString& message)
{
    auto iter = friendList.find(friendId);

    if (iter == friendList.end()) {
        return;
    }

    std::get<1>(iter.value())->setStatusMsg(message);
}

void ContentDialogManager::updateGroupStatus(int groupId)
{
    updateStatus(groupId, groupList);
}

bool ContentDialogManager::isFriendWidgetActive(int friendId)
{
    return isWidgetActive(friendId, friendList);
}

bool ContentDialogManager::isGroupWidgetActive(int groupId)
{
    return isWidgetActive(groupId, groupList);
}

/**
 * @brief Check, if user dialog is active.
 * @param id User Id.
 * @param list List with contact info.
 * @return True if user dialog is active, false otherwise.
 */
bool ContentDialogManager::isWidgetActive(int id, const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return false;
    }

    const auto dialog = std::get<0>(iter.value());
    const auto widget = std::get<1>(iter.value());
    return dialog->isActiveWidget(widget);
}

ContentDialog* ContentDialogManager::getFriendDialog(int friendId) const
{
    return getDialog(friendId, friendList);
}

ContentDialog* ContentDialogManager::getGroupDialog(int groupId) const
{
    return getDialog(groupId, groupList);
}

/**
 * @brief Update widget status and dialog title for current user.
 * @param id User Id.
 * @param list List with contact info.
 */
void ContentDialogManager::updateStatus(int id, const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return;
    }

    GenericChatroomWidget* chatroomWidget = std::get<1>(*iter);
    chatroomWidget->updateStatusLight();

    if (chatroomWidget->isActive()) {
        ContentDialog* dialog = std::get<0>(*iter);
        dialog->updateTitleAndStatusIcon();
    }
}

/**
 * @brief Select ContentDialog by id from the list.
 * @param id User Id.
 * @param list List with contact info.
 * @return ContentDialog for user and nullptr if not found.
 */
ContentDialog* ContentDialogManager::getDialog(int id, const QHash<int, ContactInfo>& list) const
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return nullptr;
    }

    return std::get<0>(iter.value());
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

    removeDialog(dialog, friendList);
    removeDialog(dialog, groupList);
}

bool ContentDialogManager::hasFriendWidget(ContentDialog* dialog, int friendId, const GenericChatroomWidget* chatroomWidget) const
{
    return dialog->hasWidget(friendId, chatroomWidget, friendList);
}

bool ContentDialogManager::hasGroupWidget(ContentDialog* dialog, int groupId, const GenericChatroomWidget* chatroomWidget) const
{
    return dialog->hasWidget(groupId, chatroomWidget, groupList);
}

FriendWidget* ContentDialogManager::getFriendWidget(int friendId) const
{
    auto iter = friendList.find(friendId);
    if (iter == friendList.end()) {
        return nullptr;
    }

    GenericChatroomWidget* widget = std::get<1>(*iter);
    return qobject_cast<FriendWidget*>(widget);
}