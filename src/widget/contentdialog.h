/*
    Copyright © 2015-2018 by The qTox Project Contributors

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

#ifndef CONTENTDIALOG_H
#define CONTENTDIALOG_H

#include "src/widget/genericchatitemlayout.h"
#include "src/widget/tool/activatedialog.h"
#include "src/core/core.h" // Status

#include <memory>
#include <tuple>

template <typename K, typename V>
class QHash;
template <typename T>
class QSet;

class ContentDialog;
class ContentLayout;
class Friend;
class FriendChatroom;
class FriendListLayout;
class FriendWidget;
class GenericChatForm;
class GenericChatroomWidget;
class Group;
class GroupChatroom;
class GroupWidget;
class QCloseEvent;
class QSplitter;
class QVBoxLayout;

using ContactInfo = std::tuple<ContentDialog*, GenericChatroomWidget*>;

class ContentDialog : public ActivateDialog
{
    Q_OBJECT
public:
    explicit ContentDialog(QWidget* parent = nullptr);
    ~ContentDialog() override;

    FriendWidget* addFriend(std::shared_ptr<FriendChatroom> chatroom, GenericChatForm* form);
    GroupWidget* addGroup(std::shared_ptr<GroupChatroom> chatroom, GenericChatForm* form);
    void removeFriend(int friendId);
    void removeGroup(int groupId);
    int chatroomWidgetCount() const;
    void ensureSplitterVisible();
    void updateTitleAndStatusIcon();

    void cycleContacts(bool forward, bool loop = true);
    void onVideoShow(QSize size);
    void onVideoHide();

    void addFriendWidget(FriendWidget* widget, Status status);
    bool isActiveWidget(GenericChatroomWidget* widget);

    bool hasFriendWidget(int friendId) const;
    bool hasGroupWidget(int groupId) const;

    void focusFriend(int friendId);
    void focusGroup(int groupId);

    bool containsFriend(int friendId) const;
    bool containsGroup(int groupId) const;

    void updateFriendStatus(int friendId, Status status);
    void updateFriendStatusLight(int friendId);
    void updateGroupStatusLight(int groupId);

    bool isFriendWidgetActive(int friendId);
    bool isGroupWidgetActive(int groupId);

    void setStatusMessage(int friendId, const QString& message);

signals:
    void friendDialogShown(const Friend* f);
    void groupDialogShown(Group* g);
    void activated();
    void willClose();

public slots:
    void reorderLayouts(bool newGroupOnTop);
    void previousContact();
    void nextContact();
    void setUsername(const QString& newName);

protected:
    bool event(QEvent* event) final override;
    void dragEnterEvent(QDragEnterEvent* event) final override;
    void dropEvent(QDropEvent* event) final override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public slots:
    void activate(GenericChatroomWidget* widget);

private slots:
    void updateFriendWidget(uint32_t friendId, QString alias);
    void onGroupchatPositionChanged(bool top);

private:
    void closeIfEmpty();
    void closeEvent(QCloseEvent* event) override;

    void retranslateUi();
    void saveDialogGeometry();
    void saveSplitterState();
    QLayout* nextLayout(QLayout* layout, bool forward) const;
    int getCurrentLayout(QLayout*& layout);
    void focusCommon(int id, QHash<int, GenericChatroomWidget*> list);

private:

    QList<QLayout*> layouts;
    QSplitter* splitter;
    FriendListLayout* friendLayout;
    GenericChatItemLayout groupLayout;
    ContentLayout* contentLayout;
    GenericChatroomWidget* activeChatroomWidget;
    QSize videoSurfaceSize;
    int videoCount;

    QHash<int, GenericChatroomWidget*> friendWidgets;
    QHash<int, GenericChatroomWidget*> groupWidgets;

    QHash<int, GenericChatForm*> friendChatForms;
    QHash<int, GenericChatForm*> groupChatForms;

    QString username;
};

#endif // CONTENTDIALOG_H
