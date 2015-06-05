/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef WIDGET_H
#define WIDGET_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QFileInfo>
#include "form/addfriendform.h"
#include "form/settingswidget.h"
#include "form/profileform.h"
#include "form/filesform.h"
#include "src/core/corestructs.h"

#define PIXELS_TO_ACT 7

namespace Ui {
class MainWindow;
}

class GenericChatroomWidget;
class Group;
class Friend;
class QSplitter;
class VideoSurface;
class QMenu;
class Core;
class Camera;
class FriendListWidget;
class MaskablePixmapWidget;
class QTimer;
class SystemTrayIcon;

class Widget : public QMainWindow
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event);

public:
    enum FilterCriteria
    {
        All=0,
        Online,
        Offline,
        Friends,
        Groups
    };
    explicit Widget(QWidget *parent = 0);
    void init();
    void setCentralWidget(QWidget *widget, const QString &widgetName);
    QString getUsername();
    Camera* getCamera();
    static Widget* getInstance();
    void newMessageAlert(GenericChatroomWidget* chat);
    bool isFriendWidgetCurActiveWidget(Friend* f);
    bool getIsWindowMinimized();
    void updateIcons();
    void clearContactsList();
    ~Widget();

    virtual void closeEvent(QCloseEvent *event);
    virtual void changeEvent(QEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

    static void confirmExecutableOpen(const QFileInfo file);

    void clearAllReceipts();
    void reloadHistory();

    void reloadTheme();
    static QString getStatusIconPath(Status status);
    static inline QIcon getStatusIcon(Status status, uint32_t w=0, uint32_t h=0);
    static QPixmap getStatusIconPixmap(Status status, uint32_t w, uint32_t h);
    static QString getStatusTitle(Status status);
    static Status getStatusFromString(QString status);

public slots:
    void onSettingsClicked();
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
    void addFriend(int friendId, const QString& userId);
    void addFriendFailed(const QString& userId, const QString& errorInfo = QString());
    void onFriendStatusChanged(int friendId, Status status);
    void onFriendStatusMessageChanged(int friendId, const QString& message);
    void onFriendUsernameChanged(int friendId, const QString& username);
    void onFriendMessageReceived(int friendId, const QString& message, bool isAction);
    void onFriendRequestReceived(const QString& userId, const QString& message);
    void onMessageSendResult(uint32_t friendId, const QString& message, int messageId);
    void onReceiptRecieved(int friendId, int receipt);
    void onEmptyGroupCreated(int groupId);
    void onGroupInviteReceived(int32_t friendId, uint8_t type, QByteArray invite);
    void onGroupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction);
    void onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t change);
    void onGroupTitleChanged(int groupnumber, const QString& author, const QString& title);
    void onGroupPeerAudioPlaying(int groupnumber, int peernumber);
    void onGroupSendResult(int groupId, const QString& message, int result);
    void playRingtone();
    void onFriendTypingChanged(int friendId, bool isTyping);
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

private slots:
    void onAddClicked();
    void onGroupClicked();
    void onTransferClicked();
    void showProfile();
    void onUsernameChanged(const QString& newUsername, const QString& oldUsername);
    void onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage);
    void onChatroomWidgetClicked(GenericChatroomWidget *);
    void removeFriend(int friendId);
    void copyFriendIdToClipboard(int friendId);
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
    void searchContacts();
    void hideFriends(QString searchString, Status status, bool hideAll = false);
    void hideGroups(QString searchString, bool hideAll = false);

private:
    enum ActiveToolMenuButton {
        AddButton,
        GroupButton,
        TransferButton,
        SettingButton,
        None,
    };
    void setActiveToolMenuButton(ActiveToolMenuButton newActiveButton);
    void hideMainForms();
    virtual bool event(QEvent * e);
    Group *createGroup(int groupId);
    void removeFriend(Friend* f, bool fake = false);
    void removeGroup(Group* g, bool fake = false);
    void saveWindowGeometry();
    void saveSplitterGeometry();
    void cycleContacts(int offset);
    void retranslateUi();
    SystemTrayIcon *icon;
    QMenu *trayMenu;
    QAction *statusOnline,
            *statusAway,
            *statusBusy,
            *actionQuit;

    Ui::MainWindow *ui;
    QSplitter *centralLayout;
    QPoint dragPosition;
    AddFriendForm *addFriendForm;
    ProfileForm *profileForm;
    SettingsWidget *settingsWidget;
    FilesForm *filesForm;
    static Widget *instance;
    GenericChatroomWidget *activeChatroomWidget;
    FriendListWidget *contactListWidget;
    MaskablePixmapWidget *profilePicture;
    bool notify(QObject *receiver, QEvent *event);
    bool autoAwayActive = false;
    Status beforeDisconnect = Status::Offline;
    QTimer *timer, *offlineMsgTimer;
    QRegExp nameMention, sanitizedNameMention;
    bool eventFlag;
    bool eventIcon;
    bool wasMaximized = false;
};

bool toxActivateEventHandler(const QByteArray& data);

#endif // WIDGET_H
