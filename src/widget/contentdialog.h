/*
    Copyright © 2015 by The qTox Project

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

#include <QDialog>
#include <tuple>
#include "src/core/corestructs.h"
#include "src/widget/genericchatitemlayout.h"

template <typename K, typename V> class QHash;
template <typename T> class QSet;

class QSplitter;
class QVBoxLayout;
class ContentLayout;
class GenericChatroomWidget;
class FriendWidget;
class GroupWidget;
class FriendListLayout;
class SettingsWidget;

class ContentDialog : public QDialog
{
    Q_OBJECT
public:
    ContentDialog(SettingsWidget* settingsWidget, QWidget* parent = 0);
    ~ContentDialog();

    FriendWidget* addFriend(int friendId, QString id);
    GroupWidget* addGroup(int groupId, const QString& name);
    void removeFriend(int friendId);
    void removeGroup(int groupId);
    bool hasFriendWidget(int friendId, GenericChatroomWidget* chatroomWidget);
    bool hasGroupWidget(int groupId, GenericChatroomWidget* chatroomWidget);
    int chatroomWidgetCount() const;
    void ensureSplitterVisible();

    void cycleContacts(bool forward, bool loop = true);

    static ContentDialog* current();
    static bool existsFriendWidget(int friendId, bool focus);
    static bool existsGroupWidget(int groupId, bool focus);
    static void updateFriendStatus(int friendId);
    static void updateFriendStatusMessage(int friendId, const QString &message);
    static void updateGroupStatus(int groupId);
    static bool isFriendWidgetActive(int friendId);
    static bool isGroupWidgetActive(int groupId);
    static ContentDialog* getFriendDialog(int friendId);
    static ContentDialog* getGroupDialog(int groupId);

public slots:
    void updateTitleUsername(const QString& username);
    void updateTitle(GenericChatroomWidget* chatroomWidget);
    void previousContact();
    void nextContact();

protected:
    bool event(QEvent* event) final override;
    void dragEnterEvent(QDragEnterEvent* event) final override;
    void dropEvent(QDropEvent* event) final override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;

private slots:
    void onChatroomWidgetClicked(GenericChatroomWidget* widget, bool group);
    void updateFriendWidget(FriendWidget* w, Status s);
    void updateGroupWidget(GroupWidget* w);
    void onGroupchatPositionChanged(bool top);

private:
    void retranslateUi();
    void saveDialogGeometry();
    void saveSplitterState();
    QLayout* nextLayout(QLayout* layout, bool forward) const;

    void remove(int id, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list);
    bool hasWidget(int id, GenericChatroomWidget* chatroomWidget, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list);
    static bool existsWidget(int id, bool focus, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list);
    static void updateStatus(int id, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list);
    static bool isWidgetActive(int id, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list);
    static ContentDialog* getDialog(int id, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list);

    QSplitter* splitter;
    FriendListLayout* friendLayout;
    GenericChatItemLayout groupLayout;
    ContentLayout* contentLayout;
    GenericChatroomWidget* activeChatroomWidget;
    GenericChatroomWidget* displayWidget = nullptr;
    SettingsWidget* settingsWidget;
    static ContentDialog* currentDialog;
    static QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> friendList;
    static QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> groupList;
};

#endif // CONTENTDIALOG_H
