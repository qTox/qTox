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
#include "widget/form/addfriendform.h"
#include "widget/form/settingswidget.h"
#include "widget/form/settings/identityform.h"
#include "widget/form/filesform.h"
#include "corestructs.h"

#define PIXELS_TO_ACT 7

namespace Ui {
class MainWindow;
}

class GenericChatroomWidget;
class Group;
struct Friend;
class QSplitter;
class SelfCamView;
class QMenu;
class Core;
class Camera;
class FriendListWidget;
class MaskablePixmapWidget;

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
    void newMessageAlert();
    bool isFriendWidgetCurActiveWidget(Friend* f);
    bool getIsWindowMinimized();
    ~Widget();

    virtual void closeEvent(QCloseEvent *event);

signals:
    void friendRequestAccepted(const QString& userId);
    void friendRequested(const QString& friendAddress, const QString& message);
    void statusSet(Status status);
    void statusSelected(Status status);
    void usernameChanged(const QString& username);
    void statusMessageChanged(const QString& statusMessage);

private slots:
    void onConnected();
    void onDisconnected();
    void onStatusSet(Status status);
    void onAddClicked();
    void onGroupClicked();
    void onTransferClicked();
    void onSettingsClicked();
    void onFailedToStartCore();
    void onBadProxyCore();
    void onAvatarClicked();
    void onSelfAvatarLoaded(const QPixmap &pic);
    void onUsernameChanged(const QString& newUsername, const QString& oldUsername);
    void onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage);
    void setUsername(const QString& username);
    void setStatusMessage(const QString &statusMessage);
    void addFriend(int friendId, const QString& userId);
    void addFriendFailed(const QString& userId);
    void onFriendStatusChanged(int friendId, Status status);
    void onFriendStatusMessageChanged(int friendId, const QString& message);
    void onFriendUsernameChanged(int friendId, const QString& username);
    void onChatroomWidgetClicked(GenericChatroomWidget *);
    void onFriendMessageReceived(int friendId, const QString& message, bool isAction);
    void onFriendRequestReceived(const QString& userId, const QString& message);
    void onEmptyGroupCreated(int groupId);
    void onGroupInviteReceived(int32_t friendId, const uint8_t *publicKey,uint16_t length);
    void onGroupMessageReceived(int groupnumber, int friendgroupnumber, const QString& message);
    void onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t change);
    void removeFriend(int friendId);
    void copyFriendIdToClipboard(int friendId);
    void removeGroup(int groupId);
    void setStatusOnline();
    void setStatusAway();
    void setStatusBusy();
    void onMessageSendResult(int friendId, const QString& message, int messageId);
    void onGroupSendResult(int groupId, const QString& message, int result);

private:
    void hideMainForms();
    virtual bool event(QEvent * e);
    Group* createGroup(int groupId);

private:
    Ui::MainWindow *ui;
    QSplitter *centralLayout;
    QPoint dragPosition;
    Core* core;
    QThread* coreThread;
    AddFriendForm friendForm;
    SettingsWidget* settingsWidget;
    FilesForm filesForm;
    static Widget* instance;
    GenericChatroomWidget* activeChatroomWidget;
    FriendListWidget* contactListWidget;
    Camera* camera;
    MaskablePixmapWidget* profilePicture;
    bool notify(QObject *receiver, QEvent *event);
};

#endif // WIDGET_H
