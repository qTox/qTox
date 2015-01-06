/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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
#include "form/addfriendform.h"
#include "form/settingswidget.h"
#include "form/settings/identityform.h"
#include "form/filesform.h"
#include "src/corestructs.h"

#define PIXELS_TO_ACT 7

namespace Ui {
class MainWindow;
}

class GenericChatroomWidget;
class Group;
struct Friend;
class QSplitter;
class VideoSurface;
class QMenu;
class Core;
class Camera;
class FriendListWidget;
class MaskablePixmapWidget;
class QTimer;
class QTranslator;

class Widget : public QMainWindow
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    void setCentralWidget(QWidget *widget, const QString &widgetName);
    QString getUsername();
    Core* getCore();
    QThread* getCoreThread();
    Camera* getCamera();
    static Widget* getInstance();
    void newMessageAlert(GenericChatroomWidget* chat);
    bool isFriendWidgetCurActiveWidget(Friend* f);
    bool getIsWindowMinimized();
    static QList<QString> searchProfiles();
    void clearContactsList();
    void setTranslation();
    void updateTrayIcon();
    Q_INVOKABLE void showWarningMsgBox(const QString& title, const QString& msg);
    Q_INVOKABLE void setEnabledThreadsafe(bool enabled);
    Q_INVOKABLE bool askQuestion(const QString& title, const QString& msg, bool defaultAns = false, bool warning = true);
    Q_INVOKABLE QString passwordDialog(const QString& cancel, const QString& body);
    // hooray for threading hacks
    Q_INVOKABLE QString askProfiles();
    ~Widget();

    virtual void closeEvent(QCloseEvent *event);
    virtual void changeEvent(QEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

    void clearAllReceipts();

    void reloadTheme();

public slots:
    void onSettingsClicked();
    void setWindowTitle(const QString& title);

signals:
    void friendRequestAccepted(const QString& userId);
    void friendRequested(const QString& friendAddress, const QString& message);
    void statusSet(Status status);
    void statusSelected(Status status);
    void usernameChanged(const QString& username);
    void statusMessageChanged(const QString& statusMessage);
    void changeProfile(const QString& profile);

private slots:
    void onConnected();
    void onDisconnected();
    void onStatusSet(Status status);
    void onAddClicked();
    void onGroupClicked();
    void onTransferClicked();
    void onFailedToStartCore();
    void onBadProxyCore();
    void onAvatarClicked();
    void onSelfAvatarLoaded(const QPixmap &pic);
    void onUsernameChanged(const QString& newUsername, const QString& oldUsername);
    void onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage);
    void setUsername(const QString& username);
    void setStatusMessage(const QString &statusMessage);
    void addFriend(int friendId, const QString& userId);
    void addFriendFailed(const QString& userId, const QString& errorInfo = QString());
    void onFriendStatusChanged(int friendId, Status status);
    void onFriendStatusMessageChanged(int friendId, const QString& message);
    void onFriendUsernameChanged(int friendId, const QString& username);
    void onChatroomWidgetClicked(GenericChatroomWidget *);
    void onFriendMessageReceived(int friendId, const QString& message, bool isAction);
    void onFriendRequestReceived(const QString& userId, const QString& message);
    void onReceiptRecieved(int friendId, int receipt);
    void onEmptyGroupCreated(int groupId);
    void onGroupInviteReceived(int32_t friendId, uint8_t type, QByteArray invite);
    void onGroupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction);
    void onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t change);
    void onGroupTitleChanged(int groupnumber, const QString& author, const QString& title);
    void removeFriend(int friendId);
    void copyFriendIdToClipboard(int friendId);
    void removeGroup(int groupId);
    void setStatusOnline();
    void setStatusAway();
    void setStatusBusy();
    void onMessageSendResult(int friendId, const QString& message, int messageId);
    void onGroupSendResult(int groupId, const QString& message, int result);
    void playRingtone();
    void onIconClick(QSystemTrayIcon::ActivationReason);
    void onUserAwayCheck();
    void onFriendTypingChanged(int friendId, bool isTyping);
    void onSetShowSystemTray(bool newValue);
    void onSplitterMoved(int pos, int index);

private:
    void init();
    void hideMainForms();
    virtual bool event(QEvent * e);
    Group* createGroup(int groupId);
    void removeFriend(Friend* f, bool fake = false);
    void removeGroup(Group* g, bool fake = false);
    void saveWindowGeometry();
    void saveSplitterGeometry();
    QString detectProfile();
    QSystemTrayIcon *icon;
    QMenu *trayMenu;
    QAction *statusOnline,
            *statusAway,
            *statusBusy,
            *actionQuit;

    Ui::MainWindow *ui;
    QSplitter *centralLayout;
    QPoint dragPosition;
    Core* core;
    QThread* coreThread;
    AddFriendForm* addFriendForm;
    SettingsWidget* settingsWidget;
    FilesForm* filesForm;
    static Widget* instance;
    GenericChatroomWidget* activeChatroomWidget;
    FriendListWidget* contactListWidget;
    MaskablePixmapWidget* profilePicture;
    bool notify(QObject *receiver, QEvent *event);
    bool autoAwayActive = false;
    Status beforeDisconnect = Status::Offline;
    QTimer* idleTimer;
    QTranslator* translator;
};

void toxActivateEventHandler(const QByteArray& data);

#endif // WIDGET_H
