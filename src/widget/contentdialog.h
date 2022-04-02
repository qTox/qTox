/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "src/core/groupid.h"
#include "src/core/toxpk.h"
#include "src/model/dialogs/idialogs.h"
#include "src/model/status.h"
#include "src/widget/genericchatitemlayout.h"
#include "src/widget/tool/activatedialog.h"

#include <memory>

template <typename K, typename V>
class QHash;

class ContentLayout;
class Core;
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
class QScrollArea;
class Settings;
class Style;
class IMessageBoxManager;
class FriendList;
class GroupList;
class Profile;

class ContentDialog : public ActivateDialog, public IDialogs
{
    Q_OBJECT
public:
    ContentDialog(const Core& core, Settings& settings, Style& style,
        IMessageBoxManager& messageBoxManager, FriendList& friendList,
        GroupList& groupList, Profile& profile, QWidget* parent = nullptr);
    ~ContentDialog() override;

    FriendWidget* addFriend(std::shared_ptr<FriendChatroom> chatroom, GenericChatForm* form);
    GroupWidget* addGroup(std::shared_ptr<GroupChatroom> chatroom, GenericChatForm* form);
    void removeFriend(const ToxPk& friendPk) override;
    void removeGroup(const GroupId& groupId) override;
    int chatroomCount() const override;
    void ensureSplitterVisible();
    void updateTitleAndStatusIcon();

    void cycleChats(bool forward, bool inverse = true);
    void onVideoShow(QSize size);
    void onVideoHide();

    void addFriendWidget(FriendWidget* widget, Status::Status status);
    bool isActiveWidget(GenericChatroomWidget* widget);

    bool hasChat(const ChatId& chatId) const override;
    bool isChatActive(const ChatId& chatId) const override;

    void focusChat(const ChatId& chatId);
    void updateFriendStatus(const ToxPk& friendPk, Status::Status status);
    void updateChatStatusLight(const ChatId& chatId);

    void setStatusMessage(const ToxPk& friendPk, const QString& message);

signals:
    void friendDialogShown(const Friend* f);
    void groupDialogShown(Group* g);
    void addFriendDialog(Friend* frnd, ContentDialog* contentDialog);
    void addGroupDialog(Group* group, ContentDialog* contentDialog);
    void activated();
    void willClose();
    void connectFriendWidget(FriendWidget& friendWidget);

public slots:
    void reorderLayouts(bool newGroupOnTop);
    void previousChat();
    void nextChat();
    void setUsername(const QString& newName);
    void reloadTheme() override;

protected:
    bool event(QEvent* event) final;
    void dragEnterEvent(QDragEnterEvent* event) final;
    void dropEvent(QDropEvent* event) final;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public slots:
    void activate(GenericChatroomWidget* widget);

private slots:
    void updateFriendWidget(const ToxPk& friendPk, QString alias);
    void onGroupchatPositionChanged(bool top);

private:
    void closeIfEmpty();
    void closeEvent(QCloseEvent* event) override;

    void retranslateUi();
    void saveDialogGeometry();
    void saveSplitterState();
    QLayout* nextLayout(QLayout* layout, bool forward) const;
    int getCurrentLayout(QLayout*& layout);
    void focusCommon(const ChatId& id, QHash<const ChatId&, GenericChatroomWidget*> list);

private:
    QList<QLayout*> layouts;
    QSplitter* splitter;
    QScrollArea* friendScroll;
    FriendListLayout* friendLayout;
    GenericChatItemLayout groupLayout;
    ContentLayout* contentLayout;
    GenericChatroomWidget* activeChatroomWidget;
    QSize videoSurfaceSize;
    int videoCount;

    QHash<const ChatId&, GenericChatroomWidget*> chatWidgets;
    QHash<const ChatId&, GenericChatForm*> chatForms;

    QString username;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;
    FriendList& friendList;
    GroupList& groupList;
    Profile& profile;
};
