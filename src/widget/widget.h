/*
    Copyright © 2014-2015 by The qTox Project

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

#ifndef WIDGET_H
#define WIDGET_H

#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QPointer>
#include <QSystemTrayIcon>
#include <QFileInfo>

#include "genericchatitemwidget.h"
#include "src/core/corestructs.h"

#define PIXELS_TO_ACT 7

class AddFriendForm;
class ContentDialog;
class ContentLayout;
class ContentWidget;
class CircleWidget;
class FilesForm;
class Friend;
class FriendListWidget;
class FriendWidget;
class GenericChatForm;
class GenericChatItemWidget;
class Group;
class GroupInviteForm;
class Core;
class Camera;
class MaskablePixmapWidget;
class ProfileForm;
class QActionGroup;
class QMenu;
class QPushButton;
class QSplitter;
class QTimer;
class SettingsWidget;
class SystemTrayIcon;
class VideoSurface;

class Widget final : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

    enum class DialogType
    {
        AddDialog,
        TransferDialog,
        SettingDialog,
        ProfileDialog,
        GroupDialog
    };

    enum class ActiveToolMenuButton {
        AddButton,
        GroupButton,
        TransferButton,
        SettingButton,
        None,
    };

    enum class FilterCriteria
    {
        All=0,
        Online,
        Offline,
        Friends,
        Groups
    };

public:
    static Widget* getInstance();

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void init();

    QString getUsername();
    Camera* getCamera();

    void showUpdateDownloadProgress();
    void addFriendDialog(Friend* frnd, ContentDialog* dialog);
    void addGroupDialog(Group* group, ContentDialog* dialog);
    bool newFriendMessageAlert(uint32_t friendId, bool sound = true);
    bool newGroupMessageAlert(int groupId, bool notify);
    bool getIsWindowMinimized();
    void updateIcons();
    void clearContactsList();

    static QString fromDialogType(DialogType type);
    ContentDialog* createContentDialog();
    ContentLayout* createContentDialog(DialogType type) const;

    static void confirmExecutableOpen(const QFileInfo &file);

    void clearAllReceipts();
    void reloadHistory();

    void reloadTheme();
    static QString getStatusIconPath(Status status);
    static inline QIcon prepareIcon(QString path, int w = 0, int h = 0);
    static QPixmap getStatusIconPixmap(QString path, uint32_t w, uint32_t h);
    static QString getStatusTitle(Status status);
    static Status getStatusFromString(QString status);

    void searchCircle(CircleWidget* circleWidget);
    void searchItem(GenericChatItemWidget* chatItem, GenericChatItemWidget::ItemType type);
    bool groupsVisible() const;

    void arrangeContent(QWidget* widget = nullptr);
    void resetIcon();

public:
    // QMainWindow overrides
    QSize minimumSizeHint() const override final;

public slots:
    void onShowSettings();
    void onSeparateWindowChanged(bool separate);
    void setWindowTitle(const QString& title);
    void forceShow();
    void onConnected();
    void onDisconnected();
    void onStatusSet(Status status);
    void onFailedToStartCore();
    void onBadProxyCore();
    void onSelfAvatarLoaded(const QPixmap &pic);
    void setUsername(const QString& username);
    void setStatusMessage(const QString &statusMessage);
    void addFriend(uint32_t friendId, const QString& userId);
    void addFriendFailed(const QString& userId, const QString& errorInfo = QString());
    void onFriendshipChanged(uint32_t friendId);
    void onFriendStatusChanged(uint32_t friendId, Status status);
    void onFriendStatusMessageChanged(uint32_t friendId, const QString& message);
    void onFriendUsernameChanged(uint32_t friendId, const QString& username);
    void onFriendAliasChanged(uint32_t friendId, QString alias);
    void onFriendMessageReceived(uint32_t friendId, const QString& message, bool isAction);
    void onFriendRequestReceived(const QString& userId, const QString& message);
    void onFriendDialogShown(Friend* f);
    void updateFriendActivity(Friend* frnd);
    void onMessageSendResult(uint32_t friendId, const QString& message, int messageId);
    void onEmptyGroupCreated(int groupId);
    void onGroupInviteReceived(uint32_t friendId, uint8_t type, QByteArray invite);
    void onGroupInviteAccepted(uint32_t friendId, uint8_t type, QByteArray invite);
    void onGroupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction);
    void onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t change);
    void onGroupTitleChanged(int groupnumber, const QString& author, const QString& title);
    void onGroupPeerAudioPlaying(int groupnumber, int peernumber);
    void onGroupSendResult(int groupId, const QString& message, int result);
    void onGroupDialogShown(Group* g);
    void nextContact();
    void previousContact();

signals:
    void friendRequestAccepted(const QString& userId);
    void friendRequested(const QString& friendAddress, const QString& message);
    void statusSet(Status status);
    void statusSelected(Status status);
    void usernameChanged(const QString& username);
    void statusMessageChanged(const QString& statusMessage);
    void resized();
#ifdef Q_OS_MAC
    void windowStateChanged(Qt::WindowStates states);
#endif

private slots:
    void onAddClicked();
    void onGroupClicked();
    void onTransferClicked();
    void showProfile();
    void onChatroomWidgetClicked(GenericChatroomWidget *, bool group);
    void onStatusMessageChanged(const QString& newStatusMessage);
    void removeFriend(uint32_t friendId);
    void copyFriendIdToClipboard(uint32_t friendId);
    void removeGroup(int groupId);
    void setStatusOnline();
    void setStatusAway();
    void setStatusBusy();
    void onIconClick(QSystemTrayIcon::ActivationReason);
    void onUserAwayCheck();
    void onEventIconTick();
    void onTryCreateTrayIcon();
    void onSetShowSystemTray(bool newValue);
    void onSplitterMoved(int pos, int index);
    void processOfflineMsgs();
    void friendListContextMenu(const QPoint &pos);
    void friendRequestsUpdate();
    void groupInvitesUpdate();
    void groupInvitesClear();
    void onDialogShown(GenericChatroomWidget *widget);

private:
    // QMainWindow overrides
    bool eventFilter(QObject *obj, QEvent *event) final override;
    bool event(QEvent * e) final override;
    void closeEvent(QCloseEvent *event) final override;
    void changeEvent(QEvent *event) final override;
    void resizeEvent(QResizeEvent *event) final override;
    void moveEvent(QMoveEvent *event) final override;

private:
    int icon_size;

private:
    bool newMessageAlert(QWidget* currentWindow, bool isActive, bool sound = true, bool notify = true);
    void setActiveToolMenuButton(ActiveToolMenuButton newActiveButton);
    Group *createGroup(int groupId);
    void removeFriend(Friend* f, bool fake = false);
    void removeGroup(Group* g, bool fake = false);
    void saveWindowGeometry();
    void saveSplitterGeometry();
    void cycleContacts(bool forward);
    void searchContacts();
    void changeDisplayMode();
    void updateFilterText();
    FilterCriteria getFilterCriteria() const;
    static bool filterGroups(FilterCriteria filter);
    static bool filterOnline(FilterCriteria filter);
    static bool filterOffline(FilterCriteria filter);
    void retranslateUi();
    void focusChatInput();
    void showContentWidget(QWidget* widget, const QString& title = QString(),
                  ActiveToolMenuButton activeButton = ActiveToolMenuButton::None);

private:
    static Widget *instance;

    SystemTrayIcon *icon = nullptr;
    QMenu *trayMenu;
    QAction *statusOnline;
    QAction *statusAway;
    QAction *statusBusy;
    QAction *actionLogout;
    QAction *actionQuit;
    QAction *actionShow;

    QMenu* filterMenu;
    QActionGroup* filterGroup;
    QAction* filterAllAction;
    QAction* filterOnlineAction;
    QAction* filterOfflineAction;
    QAction* filterFriendsAction;
    QAction* filterGroupsAction;
    QActionGroup* filterDisplayGroup;
    QAction* filterDisplayName;
    QAction* filterDisplayActivity;

    QPoint dragPosition;

    Qt::Edge contentArrangement;
    QPointer<QWidget> contentWidget;
    QPointer<AddFriendForm> addFriendForm;
    QPointer<GroupInviteForm> groupInviteForm;
    QPointer<ContentWidget> profileForm;
    QPointer<SettingsWidget> settingsWidget;
    QPointer<FilesForm> filesForm;
    QPointer<GenericChatForm> activeChat;

    FriendListWidget* contactListWidget;
    MaskablePixmapWidget* profilePicture;

    bool notify(QObject *receiver, QEvent *event);
    bool autoAwayActive = false;
    QTimer *timer, *offlineMsgTimer;
    QRegExp nameMention, sanitizedNameMention;
    bool eventFlag;
    bool eventIcon;
    bool wasMaximized = false;
    QPushButton* friendRequestsButton;
    QPushButton* groupInvitesButton;
    unsigned int unreadGroupInvites;

    QMap<int, FriendWidget*> friendWidgets;

#ifdef Q_OS_MAC
    QAction* fileMenu;
    QAction* editMenu;
    QAction* contactMenu;
    QMenu* changeStatusMenu;
    QAction* editProfileAction;
    QAction* logoutAction;
    QAction* addContactAction;
    QAction* nextConversationAction;
    QAction* previousConversationAction;
#endif
};

bool toxActivateEventHandler(const QByteArray& data);

#endif // WIDGET_H
