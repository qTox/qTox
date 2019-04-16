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
void removeDialog(ContentDialog* dialog, QHash<const ContactId&, ContentDialog*>& dialogs)
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

bool ContentDialogManager::contactWidgetExists(const ContactId& contactId)
{
    const auto dialog = contactDialogs.value(contactId, nullptr);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->containsContact(contactId);
}

FriendWidget* ContentDialogManager::addFriendToDialog(ContentDialog* dialog,
    std::shared_ptr<FriendChatroom> chatroom, GenericChatForm* form)
{
    auto friendWidget = dialog->addFriend(chatroom, form);
    const auto friendPk = friendWidget->getFriend()->getPublicKey();

    ContentDialog* lastDialog = getFriendDialog(friendPk);
    if (lastDialog) {
        lastDialog->removeFriend(friendPk);
    }

    contactDialogs[friendPk] = dialog;
    return friendWidget;
}

GroupWidget* ContentDialogManager::addGroupToDialog(ContentDialog* dialog,
    std::shared_ptr<GroupChatroom> chatroom, GenericChatForm* form)
{
    auto groupWidget = dialog->addGroup(chatroom, form);
    const auto& groupId = groupWidget->getGroup()->getPersistentId();

    ContentDialog* lastDialog = getGroupDialog(groupId);
    if (lastDialog) {
        lastDialog->removeGroup(groupId);
    }

    contactDialogs[groupId] = dialog;
    return groupWidget;
}

void ContentDialogManager::focusContact(const ContactId& contactId)
{
    auto dialog = focusDialog(contactId, contactDialogs);
    if (dialog != nullptr) {
        dialog->focusContact(contactId);
    }
}

/**
 * @brief Focus the dialog if it exists.
 * @param id User Id.
 * @param list List with dialogs
 * @return ContentDialog if found, nullptr otherwise
 */
ContentDialog* ContentDialogManager::focusDialog(const ContactId& id, const QHash<const ContactId&, ContentDialog*>& list)
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
    auto dialog = contactDialogs.value(friendPk);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateContactStatusLight(friendPk);
    if (dialog->isContactWidgetActive(friendPk)) {
        dialog->updateTitleAndStatusIcon();
    }

    Friend* f = FriendList::findFriend(friendPk);
    dialog->updateFriendStatus(friendPk, f->getStatus());
}

void ContentDialogManager::updateGroupStatus(const GroupId& groupId)
{
    auto dialog = contactDialogs.value(groupId);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateContactStatusLight(groupId);
    if (dialog->isContactWidgetActive(groupId)) {
        dialog->updateTitleAndStatusIcon();
    }
}

bool ContentDialogManager::isContactWidgetActive(const ContactId& contactId)
{
    const auto dialog = contactDialogs.value(contactId);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->isContactWidgetActive(contactId);
}

ContentDialog* ContentDialogManager::getFriendDialog(const ToxPk& friendPk) const
{
    return contactDialogs.value(friendPk);
}

ContentDialog* ContentDialogManager::getGroupDialog(const GroupId& groupId) const
{
    return contactDialogs.value(groupId);
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

    removeDialog(dialog, contactDialogs);
}
