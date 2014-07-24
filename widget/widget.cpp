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

#include "widget.h"
#include "ui_widget.h"
#include "settings.h"
#include "friend.h"
#include "friendlist.h"
#include "widget/tool/friendrequestdialog.h"
#include "widget/friendwidget.h"
#include "grouplist.h"
#include "group.h"
#include "widget/groupwidget.h"
#include "widget/form/groupchatform.h"
#include <QMessageBox>
#include <QDebug>
#include <QSound>
#include <QTextStream>
#include <QFile>
#include <QString>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QCursor>
#include <QSettings>
#include <QClipboard>

Widget *Widget::instance{nullptr};

Widget::Widget(QWidget *parent) :
    QWidget(parent), ui(new Ui::Widget), activeFriendWidget{nullptr}, activeGroupWidget{nullptr}
{
    ui->setupUi(this);

    QSettings settings(Settings::getInstance().getSettingsDirPath() + '/' + "windowSettings.ini", QSettings::IniFormat);
    if (!settings.contains("useNativeTheme"))
        useNativeTheme = 1;
    else
        useNativeTheme = settings.value("useNativeTheme").toInt();

    if (useNativeTheme)
    {
        ui->titleBar->hide();
        this->layout()->setContentsMargins(0, 0, 0, 0);

        QString friendListStylesheet = "";
        try
        {
            QFile f(":ui/friendList/friendList.css");
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream friendListStylesheetStream(&f);
            friendListStylesheet = friendListStylesheetStream.readAll();
        }
        catch (int e) {}
        ui->friendList->setObjectName("friendList");
        ui->friendList->setStyleSheet(friendListStylesheet);
    }
    else
    {
        QString windowStylesheet = "";
        try
        {
            QFile f(":ui/window/window.css");
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream windowStylesheetStream(&f);
            windowStylesheet = windowStylesheetStream.readAll();
        }
        catch (int e) {}
        this->setObjectName("activeWindow");
        this->setStyleSheet(windowStylesheet);
        ui->statusPanel->setStyleSheet(QString(""));
        ui->friendList->setStyleSheet(QString(""));

        QString friendListStylesheet = "";
        try
        {
            QFile f(":ui/friendList/friendList.css");
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream friendListStylesheetStream(&f);
            friendListStylesheet = friendListStylesheetStream.readAll();
        }
        catch (int e) {}
        ui->friendList->setObjectName("friendList");
        ui->friendList->setStyleSheet(friendListStylesheet);

        ui->tbMenu->setIcon(QIcon(":ui/window/applicationIcon.png"));
        ui->pbMin->setObjectName("minimizeButton");
        ui->pbMax->setObjectName("maximizeButton");
        ui->pbClose->setObjectName("closeButton");

        setWindowFlags(Qt::CustomizeWindowHint);
        setWindowFlags(Qt::FramelessWindowHint);

        addAction(ui->actionClose);

        connect(ui->pbMin, SIGNAL(clicked()), this, SLOT(minimizeBtnClicked()));
        connect(ui->pbMax, SIGNAL(clicked()), this, SLOT(maximizeBtnClicked()));
        connect(ui->pbClose, SIGNAL(clicked()), this, SLOT(close()));

        m_titleMode = FullTitle;
        moveWidget = false;
        inResizeZone = false;
        allowToResize = false;
        resizeVerSup = false;
        resizeHorEsq = false;
        resizeDiagSupEsq = false;
        resizeDiagSupDer = false;

    QSettings settings(Settings::getInstance().getSettingsDirPath() + '/' + "windowSettings.ini", QSettings::IniFormat);
    QRect geo = settings.value("geometry").toRect();

        if (geo.height() > 0 and geo.x() < QApplication::desktop()->width() and geo.width() > 0 and geo.y() < QApplication::desktop()->height())
            this->setGeometry(geo);

        if (settings.value("maximized").toBool())
        {
            showMaximized();
            ui->pbMax->setObjectName("restoreButton");
        }

        QList<QWidget*> widgets = this->findChildren<QWidget*>();

        foreach (QWidget *widget, widgets)
        {
            widget->setMouseTracking(true);
        }
    }

    isWindowMinimized = 0;

    ui->mainContent->setLayout(new QVBoxLayout());
    ui->mainHead->setLayout(new QVBoxLayout());
    ui->mainHead->layout()->setMargin(0);
    ui->mainHead->layout()->setSpacing(0);
    QWidget* friendListWidget = new QWidget();
    friendListWidget->setLayout(new QVBoxLayout());
    friendListWidget->layout()->setSpacing(0);
    friendListWidget->layout()->setMargin(0);
    friendListWidget->setLayoutDirection(Qt::LeftToRight);
    ui->friendList->setWidget(friendListWidget);

    // delay setting username and message until Core inits
    //ui->nameLabel->setText(core->getUsername());
    ui->nameLabel->label->setStyleSheet("QLabel { color : white; font-size: 11pt; font-weight:bold;}");
    //ui->statusLabel->setText(core->getStatusMessage());
    ui->statusLabel->label->setStyleSheet("QLabel { color : white; font-size: 8pt;}");
    ui->friendList->widget()->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QFile f1(":/ui/statusButton/statusButton.css");
    f1.open(QFile::ReadOnly | QFile::Text);
    QTextStream statusButtonStylesheetStream(&f1);
    ui->statusButton->setStyleSheet(statusButtonStylesheetStream.readAll());

    QMenu *statusButtonMenu = new QMenu(ui->statusButton);
    QAction* setStatusOnline = statusButtonMenu->addAction(tr("Online","Button to set your status to 'Online'"));
    setStatusOnline->setIcon(QIcon(":ui/statusButton/dot_online.png"));
    QAction* setStatusAway = statusButtonMenu->addAction(tr("Away","Button to set your status to 'Away'"));
    setStatusAway->setIcon(QIcon(":ui/statusButton/dot_idle.png"));
    QAction* setStatusBusy = statusButtonMenu->addAction(tr("Busy","Button to set your status to 'Busy'"));
    setStatusBusy->setIcon(QIcon(":ui/statusButton/dot_busy.png"));
    ui->statusButton->setMenu(statusButtonMenu);


    this->setMouseTracking(true);

    QList<QWidget*> widgets = this->findChildren<QWidget*>();
    foreach (QWidget *widget, widgets)
        widget->setMouseTracking(true);

    ui->titleBar->setMouseTracking(true);
    ui->LTitle->setMouseTracking(true);
    ui->tbMenu->setMouseTracking(true);
    ui->pbMin->setMouseTracking(true);
    ui->pbMax->setMouseTracking(true);
    ui->pbClose->setMouseTracking(true);
    ui->statusHead->setMouseTracking(true);

    ui->friendList->viewport()->installEventFilter(this);

    QList<int> currentSizes = ui->centralWidget->sizes();
    currentSizes[0] = 225;
    ui->centralWidget->setSizes(currentSizes);

    ui->statusButton->setObjectName("offline");
    ui->statusButton->style()->polish(ui->statusButton);

    camera = new Camera;
    camview = new SelfCamView(camera);

    qRegisterMetaType<Status>("Status");
    qRegisterMetaType<vpx_image>("vpx_image");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<ToxFile>("ToxFile");
    qRegisterMetaType<ToxFile::FileDirection>("ToxFile::FileDirection");

    coreThread = new QThread(this);
    core = new Core(camera, coreThread);
    core->moveToThread(coreThread);
    connect(coreThread, &QThread::started, core, &Core::start);

    connect(core, &Core::connected, this, &Widget::onConnected);
    connect(core, &Core::disconnected, this, &Widget::onDisconnected);
    connect(core, &Core::failedToStart, this, &Widget::onFailedToStartCore);
    connect(core, &Core::statusSet, this, &Widget::onStatusSet);
    connect(core, &Core::usernameSet, this, &Widget::setUsername);
    connect(core, &Core::statusMessageSet, this, &Widget::setStatusMessage);
    connect(core, &Core::friendAddressGenerated, &settingsForm, &SettingsForm::setFriendAddress);
    connect(core, SIGNAL(fileDownloadFinished(const QString&)), &filesForm, SLOT(onFileDownloadComplete(const QString&)));
    connect(core, SIGNAL(fileUploadFinished(const QString&)), &filesForm, SLOT(onFileUploadComplete(const QString&)));
    connect(core, &Core::friendAdded, this, &Widget::addFriend);
    connect(core, &Core::failedToAddFriend, this, &Widget::addFriendFailed);
    connect(core, &Core::friendStatusChanged, this, &Widget::onFriendStatusChanged);
    connect(core, &Core::friendUsernameChanged, this, &Widget::onFriendUsernameChanged);
    connect(core, &Core::friendStatusChanged, this, &Widget::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, this, &Widget::onFriendStatusMessageChanged);
    connect(core, &Core::friendUsernameLoaded, this, &Widget::onFriendUsernameLoaded);
    connect(core, &Core::friendStatusMessageLoaded, this, &Widget::onFriendStatusMessageLoaded);
    connect(core, &Core::friendRequestReceived, this, &Widget::onFriendRequestReceived);
    connect(core, &Core::friendMessageReceived, this, &Widget::onFriendMessageReceived);
    connect(core, &Core::groupInviteReceived, this, &Widget::onGroupInviteReceived);
    connect(core, &Core::groupMessageReceived, this, &Widget::onGroupMessageReceived);
    connect(core, &Core::groupNamelistChanged, this, &Widget::onGroupNamelistChanged);
    connect(core, &Core::emptyGroupCreated, this, &Widget::onEmptyGroupCreated);

    connect(this, &Widget::statusSet, core, &Core::setStatus);
    connect(this, &Widget::friendRequested, core, &Core::requestFriendship);
    connect(this, &Widget::friendRequestAccepted, core, &Core::acceptFriendRequest);

    connect(ui->centralWidget, SIGNAL(splitterMoved(int,int)),this, SLOT(splitterMoved(int,int)));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(onAddClicked()));
    connect(ui->groupButton, SIGNAL(clicked()), this, SLOT(onGroupClicked()));
    connect(ui->transferButton, SIGNAL(clicked()), this, SLOT(onTransferClicked()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onSettingsClicked()));
    connect(ui->nameLabel, SIGNAL(textChanged(QString,QString)), this, SLOT(onUsernameChanged(QString,QString)));
    connect(ui->statusLabel, SIGNAL(textChanged(QString,QString)), this, SLOT(onStatusMessageChanged(QString,QString)));
    connect(setStatusOnline, SIGNAL(triggered()), this, SLOT(setStatusOnline()));
    connect(setStatusAway, SIGNAL(triggered()), this, SLOT(setStatusAway()));
    connect(setStatusBusy, SIGNAL(triggered()), this, SLOT(setStatusBusy()));
    connect(&settingsForm.name, SIGNAL(editingFinished()), this, SLOT(onUsernameChanged()));
    connect(&settingsForm.statusText, SIGNAL(editingFinished()), this, SLOT(onStatusMessageChanged()));
    connect(&friendForm, SIGNAL(friendRequested(QString,QString)), this, SIGNAL(friendRequested(QString,QString)));

    coreThread->start();

    friendForm.show(*ui);
    isFriendWidgetActive = 0;
    isGroupWidgetActive = 0;
}

Widget::~Widget()
{
    core->saveConfiguration();
    instance = nullptr;
    coreThread->exit();
    coreThread->wait();
    delete core;
    delete camview;

    hideMainForms();

    for (Friend* f : FriendList::friendList)
        delete f;
    FriendList::friendList.clear();
    for (Group* g : GroupList::groupList)
        delete g;
    GroupList::groupList.clear();
    QSettings settings(Settings::getInstance().getSettingsDirPath() + '/' + "windowSettings.ini", QSettings::IniFormat);
    settings.setValue("geometry", geometry());
    settings.setValue("maximized", isMaximized());
    settings.setValue("useNativeTheme", useNativeTheme);
    delete ui;
}

Widget* Widget::getInstance()
{
    if (!instance)
        instance = new Widget();
    return instance;
}

//Super ugly hack to enable resizable friend widgets
//There should be a way to set them to resize automagicly, but I can't seem to find it.
void Widget::splitterMoved(int, int)
{
    updateFriendListWidth();
}

QThread* Widget::getCoreThread()
{
    return coreThread;
}

void Widget::updateFriendListWidth()
{
    int newWidth = ui->friendList->width();
    for (Friend* f : FriendList::friendList)
        if (f->widget != nullptr)
            f->widget->setNewFixedWidth(newWidth);
    for (Group* g : GroupList::groupList)
        if (g->widget != nullptr)
            g->widget->setNewFixedWidth(newWidth);
}

QString Widget::getUsername()
{
    return ui->nameLabel->text();
}

Camera* Widget::getCamera()
{
    return camera;
}

void Widget::onConnected()
{
    emit statusSet(Status::Online);
}

void Widget::onDisconnected()
{
    emit statusSet(Status::Offline);
}

void Widget::onFailedToStartCore()
{
    QMessageBox critical(this);
    critical.setText("Toxcore failed to start, the application will terminate after you close this message.");
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    qApp->quit();
}

void Widget::onStatusSet(Status status)
{
    //We have to use stylesheets here, there's no way to
    //prevent the button icon from moving when pressed otherwise
    if (status == Status::Online)
    {
        ui->statusButton->setObjectName("online");
        ui->statusButton->style()->polish(ui->statusButton);
    }
    else if (status == Status::Away)
    {
        ui->statusButton->setObjectName("away");
        ui->statusButton->style()->polish(ui->statusButton);
    }
    else if (status == Status::Busy)
    {
        ui->statusButton->setObjectName("busy");
        ui->statusButton->style()->polish(ui->statusButton);
    }
    else if (status == Status::Offline)
    {
        ui->statusButton->setObjectName("offline");
        ui->statusButton->style()->polish(ui->statusButton);
    }
}

void Widget::onAddClicked()
{
    hideMainForms();
    friendForm.show(*ui);
}

void Widget::onGroupClicked()
{
    core->createGroup();
}

void Widget::onTransferClicked()
{
    hideMainForms();
    filesForm.show(*ui);
    isFriendWidgetActive = 0;
    isGroupWidgetActive = 0;
}

void Widget::onSettingsClicked()
{
    hideMainForms();
    settingsForm.show(*ui);
    isFriendWidgetActive = 0;
    isGroupWidgetActive = 0;
}

void Widget::hideMainForms()
{
    QLayoutItem* item;
    while ((item = ui->mainHead->layout()->takeAt(0)) != 0)
        item->widget()->hide();
    while ((item = ui->mainContent->layout()->takeAt(0)) != 0)
        item->widget()->hide();
    
    if (activeFriendWidget != nullptr)
    {
        Friend* f = FriendList::findFriend(activeFriendWidget->friendId);
        if (f != nullptr)
            activeFriendWidget->setAsInactiveChatroom();
    }
    if (activeGroupWidget != nullptr)
    {
        Group* g = GroupList::findGroup(activeGroupWidget->groupId);
        if (g != nullptr)
            activeGroupWidget->setAsInactiveChatroom();
    }
}

void Widget::onUsernameChanged()
{
    const QString newUsername = settingsForm.name.text();
    ui->nameLabel->setText(newUsername);
    ui->nameLabel->setToolTip(newUsername); // for overlength names
    settingsForm.name.setText(newUsername);
    core->setUsername(newUsername);
}

void Widget::onUsernameChanged(const QString& newUsername, const QString& oldUsername)
{
    ui->nameLabel->setText(oldUsername); // restore old username until Core tells us to set it
    ui->nameLabel->setToolTip(oldUsername); // for overlength names
    settingsForm.name.setText(oldUsername);
    core->setUsername(newUsername);
}

void Widget::setUsername(const QString& username)
{
    ui->nameLabel->setText(username);
    ui->nameLabel->setToolTip(username); // for overlength names
    settingsForm.name.setText(username);
}

void Widget::onStatusMessageChanged()
{
    const QString newStatusMessage = settingsForm.statusText.text();
    ui->statusLabel->setText(newStatusMessage);
    ui->statusLabel->setToolTip(newStatusMessage); // for overlength messsages
    settingsForm.statusText.setText(newStatusMessage);
    core->setStatusMessage(newStatusMessage);
}

void Widget::onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage)
{
    ui->statusLabel->setText(oldStatusMessage); // restore old status message until Core tells us to set it
    ui->statusLabel->setToolTip(oldStatusMessage); // for overlength messsages
    settingsForm.statusText.setText(oldStatusMessage);
    core->setStatusMessage(newStatusMessage);
}

void Widget::setStatusMessage(const QString &statusMessage)
{
    ui->statusLabel->setText(statusMessage);
    ui->statusLabel->setToolTip(statusMessage); // for overlength messsages
    settingsForm.statusText.setText(statusMessage);
}

void Widget::addFriend(int friendId, const QString &userId)
{
    qDebug() << "Adding friend with id "+userId;
    Friend* newfriend = FriendList::addFriend(friendId, userId);
    QWidget* widget = ui->friendList->widget();
    QLayout* layout = widget->layout();
    layout->addWidget(newfriend->widget);
    updateFriendListWidth();
    connect(newfriend->widget, SIGNAL(friendWidgetClicked(FriendWidget*)), this, SLOT(onFriendWidgetClicked(FriendWidget*)));
    connect(newfriend->widget, SIGNAL(removeFriend(int)), this, SLOT(removeFriend(int)));
    connect(newfriend->widget, SIGNAL(copyFriendIdToClipboard(int)), this, SLOT(copyFriendIdToClipboard(int)));
    connect(newfriend->chatForm, SIGNAL(sendMessage(int,QString)), core, SLOT(sendMessage(int,QString)));
    connect(newfriend->chatForm, SIGNAL(sendFile(int32_t, QString, QString, long long)), core, SLOT(sendFile(int32_t, QString, QString, long long)));
    connect(newfriend->chatForm, SIGNAL(answerCall(int)), core, SLOT(answerCall(int)));
    connect(newfriend->chatForm, SIGNAL(hangupCall(int)), core, SLOT(hangupCall(int)));
    connect(newfriend->chatForm, SIGNAL(startCall(int)), core, SLOT(startCall(int)));
    connect(newfriend->chatForm, SIGNAL(startVideoCall(int,bool)), core, SLOT(startCall(int,bool)));
    connect(newfriend->chatForm, SIGNAL(cancelCall(int,int)), core, SLOT(cancelCall(int,int)));
    connect(core, &Core::fileReceiveRequested, newfriend->chatForm, &ChatForm::onFileRecvRequest);
    connect(core, &Core::avInvite, newfriend->chatForm, &ChatForm::onAvInvite);
    connect(core, &Core::avStart, newfriend->chatForm, &ChatForm::onAvStart);
    connect(core, &Core::avCancel, newfriend->chatForm, &ChatForm::onAvCancel);
    connect(core, &Core::avEnd, newfriend->chatForm, &ChatForm::onAvEnd);
    connect(core, &Core::avRinging, newfriend->chatForm, &ChatForm::onAvRinging);
    connect(core, &Core::avStarting, newfriend->chatForm, &ChatForm::onAvStarting);
    connect(core, &Core::avEnding, newfriend->chatForm, &ChatForm::onAvEnding);
    connect(core, &Core::avRequestTimeout, newfriend->chatForm, &ChatForm::onAvRequestTimeout);
    connect(core, &Core::avPeerTimeout, newfriend->chatForm, &ChatForm::onAvPeerTimeout);
}

void Widget::addFriendFailed(const QString&)
{
    QMessageBox::critical(0,"Error","Couldn't request friendship");
}

void Widget::onFriendStatusChanged(int friendId, Status status)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->friendStatus = status;
    updateFriendStatusLights(friendId);
}

void Widget::onFriendStatusMessageChanged(int friendId, const QString& message)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->setStatusMessage(message);
}

void Widget::onFriendUsernameChanged(int friendId, const QString& username)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->setName(username);
}

void Widget::onFriendStatusMessageLoaded(int friendId, const QString& message)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->setStatusMessage(message);
}

void Widget::onFriendUsernameLoaded(int friendId, const QString& username)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->setName(username);
}

void Widget::onFriendWidgetClicked(FriendWidget *widget)
{
    Friend* f = FriendList::findFriend(widget->friendId);
    if (!f)
        return;

    hideMainForms();
    f->chatForm->show(*ui);
    if (activeFriendWidget != nullptr)
    {
        activeFriendWidget->setAsInactiveChatroom();
    }
    activeFriendWidget = widget;
    widget->setAsActiveChatroom();
    isFriendWidgetActive = 1;
    isGroupWidgetActive = 0;

    if (f->hasNewMessages != 0)
        f->hasNewMessages = 0;

    updateFriendStatusLights(f->friendId);
}

void Widget::onFriendMessageReceived(int friendId, const QString& message)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->chatForm->addFriendMessage(message);

    if (activeFriendWidget != nullptr)
    {
        Friend* f2 = FriendList::findFriend(activeFriendWidget->friendId);
        if (((f->friendId != f2->friendId) || isFriendWidgetActive == 0) || isWindowMinimized || !isActiveWindow())
        {
            f->hasNewMessages = 1;
            newMessageAlert();
        }
    }
    else
    {
        f->hasNewMessages = 1;
        newMessageAlert();
    }

    updateFriendStatusLights(friendId);
}

void Widget::updateFriendStatusLights(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    Status status = f->friendStatus;
    if (status == Status::Online && f->hasNewMessages == 0)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_online.png"));
    else if (status == Status::Online && f->hasNewMessages == 1)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.png"));
    else if (status == Status::Away && f->hasNewMessages == 0)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_idle.png"));
    else if (status == Status::Away && f->hasNewMessages == 1)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_idle_notification.png"));
    else if (status == Status::Busy && f->hasNewMessages == 0)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_busy.png"));
    else if (status == Status::Busy && f->hasNewMessages == 1)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_busy_notification.png"));
    else if (status == Status::Offline && f->hasNewMessages == 0)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_away.png"));
    else if (status == Status::Offline && f->hasNewMessages == 1)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_away_notification.png"));
}

void Widget::newMessageAlert()
{
    QApplication::alert(this);
    QSound::play(":audio/notification.wav");
}

void Widget::onFriendRequestReceived(const QString& userId, const QString& message)
{
    FriendRequestDialog dialog(this, userId, message);

    if (dialog.exec() == QDialog::Accepted)
        emit friendRequestAccepted(userId);
}

void Widget::removeFriend(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    f->widget->setAsInactiveChatroom();
    if (f->widget == activeFriendWidget)
        activeFriendWidget = nullptr;
    FriendList::removeFriend(friendId);
    core->removeFriend(friendId);
    delete f;
    if (ui->mainHead->layout()->isEmpty())
        onAddClicked();
}

void Widget::copyFriendIdToClipboard(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    if (f != nullptr)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(f->userId, QClipboard::Clipboard);
    }
}

void Widget::onGroupInviteReceived(int32_t friendId, const uint8_t* publicKey)
{
    int groupId = core->joinGroupchat(friendId, publicKey);
    if (groupId == -1)
    {
        qWarning() << "Widget::onGroupInviteReceived: Unable to accept invitation";
        return;
    }
}

void Widget::onGroupMessageReceived(int groupnumber, int friendgroupnumber, const QString& message)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    g->chatForm->addGroupMessage(message, friendgroupnumber);

    if ((isGroupWidgetActive != 1 || (g->groupId != activeGroupWidget->groupId)) || isWindowMinimized || !isActiveWindow())
    {
        if (message.contains(core->getUsername(), Qt::CaseInsensitive))
        {
            newMessageAlert();
            g->hasNewMessages = 1;
            g->userWasMentioned = 1;
            if (useNativeTheme)
                g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.png"));
            else
                g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_groupchat_notification.png"));
        }
        else
            if (g->hasNewMessages == 0)
            {
                g->hasNewMessages = 1;
                if (useNativeTheme)
                    g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.png"));
                else
                    g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_groupchat_newmessages.png"));
            }
    }
}

void Widget::onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t Change)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
    {
        qDebug() << "Widget::onGroupNamelistChanged: Group not found, creating it";
        g = createGroup(groupnumber);
    }

    TOX_CHAT_CHANGE change = static_cast<TOX_CHAT_CHANGE>(Change);
    if (change == TOX_CHAT_CHANGE_PEER_ADD)
        g->addPeer(peernumber,"<Unknown>");
    else if (change == TOX_CHAT_CHANGE_PEER_DEL)
        g->removePeer(peernumber);
    else if (change == TOX_CHAT_CHANGE_PEER_NAME)
        g->updatePeer(peernumber,core->getGroupPeerName(groupnumber, peernumber));
}

void Widget::onGroupWidgetClicked(GroupWidget* widget)
{
    Group* g = GroupList::findGroup(widget->groupId);
    if (!g)
        return;

    hideMainForms();
    g->chatForm->show(*ui);
    if (activeGroupWidget != nullptr)
    {
        activeGroupWidget->setAsInactiveChatroom();
    }
    activeGroupWidget = widget;
    widget->setAsActiveChatroom();
    isFriendWidgetActive = 0;
    isGroupWidgetActive = 1;

    if (g->hasNewMessages != 0)
    {
        g->hasNewMessages = 0;
        g->userWasMentioned = 0;
        if (useNativeTheme)
            g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_online.png"));
        else
            g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_groupchat.png"));
    }
}

void Widget::removeGroup(int groupId)
{
    Group* g = GroupList::findGroup(groupId);
    g->widget->setAsInactiveChatroom();
    if (g->widget == activeGroupWidget)
        activeGroupWidget = nullptr;
    GroupList::removeGroup(groupId);
    core->removeGroup(groupId);
    delete g;
    if (ui->mainHead->layout()->isEmpty())
        onAddClicked();
}

Core *Widget::getCore()
{
    return core;
}

Group *Widget::createGroup(int groupId)
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
    {
        qWarning() << "Widget::createGroup: Group already exists";
        return g;
    }

    QString groupName = QString("Groupchat #%1").arg(groupId);
    Group* newgroup = GroupList::addGroup(groupId, groupName);
    QWidget* widget = ui->friendList->widget();
    QLayout* layout = widget->layout();
    layout->addWidget(newgroup->widget);
    if (!useNativeTheme)
        newgroup->widget->statusPic.setPixmap(QPixmap(":img/status/dot_groupchat.png"));
    updateFriendListWidth();
    connect(newgroup->widget, SIGNAL(groupWidgetClicked(GroupWidget*)), this, SLOT(onGroupWidgetClicked(GroupWidget*)));
    connect(newgroup->widget, SIGNAL(removeGroup(int)), this, SLOT(removeGroup(int)));
    connect(newgroup->chatForm, SIGNAL(sendMessage(int,QString)), core, SLOT(sendGroupMessage(int,QString)));
    return newgroup;
}

void Widget::showTestCamview()
{
    camview->show();
}

void Widget::onEmptyGroupCreated(int groupId)
{
    createGroup(groupId);
}

bool Widget::isFriendWidgetCurActiveWidget(Friend* f)
{
    if (!f)
        return false;
    if (activeFriendWidget != nullptr)
    {
        Friend* f2 = FriendList::findFriend(activeFriendWidget->friendId);
        if ((f->friendId != f2->friendId) || isFriendWidgetActive == 0)
            return false;
    }
    else
        return false;
    return true;
}

void Widget::resizeEvent(QResizeEvent *)
{
    updateFriendListWidth();
}


bool Widget::event(QEvent * e)
{

    if( e->type() == QEvent::WindowStateChange )
    {
        if(windowState().testFlag(Qt::WindowMinimized) == true)
        {
            isWindowMinimized = 1;
        }
    }
    else if (e->type() == QEvent::WindowActivate)
    {
        if (!useNativeTheme)
        {
            this->setObjectName("activeWindow");
            this->style()->polish(this);
        }
        isWindowMinimized = 0;
        if (isFriendWidgetActive && activeFriendWidget != nullptr)
        {
            Friend* f = FriendList::findFriend(activeFriendWidget->friendId);
            f->hasNewMessages = 0;
            updateFriendStatusLights(f->friendId);
        }
        else if (isGroupWidgetActive && activeGroupWidget != nullptr)
        {
            Group* g = GroupList::findGroup(activeGroupWidget->groupId);
            g->hasNewMessages = 0;
            g->userWasMentioned = 0;
            if (useNativeTheme)
                g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_online.png"));
            else
                g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_groupchat.png"));
        }
    }
    else if (e->type() == QEvent::WindowDeactivate && !useNativeTheme)
    {
        this->setObjectName("inactiveWindow");
        this->style()->polish(this);
    }
    else if (e->type() == QEvent::MouseMove && !useNativeTheme)
    {
        QMouseEvent *k = (QMouseEvent *)e;
        int xMouse = k->pos().x();
        int yMouse = k->pos().y();
        int wWidth = this->geometry().width();
        int wHeight = this->geometry().height();

        if (moveWidget)
        {
            inResizeZone = false;
            moveWindow(k);
        }
        else if (allowToResize)
            resizeWindow(k);
        else if (xMouse >= wWidth - PIXELS_TO_ACT or allowToResize)
        {
            inResizeZone = true;

            if (yMouse >= wHeight - PIXELS_TO_ACT)
            {
                setCursor(Qt::SizeFDiagCursor);
                resizeWindow(k);
            }
            else if (yMouse <= PIXELS_TO_ACT)
            {
                setCursor(Qt::SizeBDiagCursor);
                resizeWindow(k);
            }

        }
        else
        {
            inResizeZone = false;
            setCursor(Qt::ArrowCursor);
        }

        e->accept();
    }

    return QWidget::event(e);
}

void Widget::mousePressEvent(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        if (e->button() == Qt::LeftButton)
        {
            if (inResizeZone)
            {
                allowToResize = true;

                if (e->pos().y() <= PIXELS_TO_ACT)
                {
                    if (e->pos().x() <= PIXELS_TO_ACT)
                        resizeDiagSupEsq = true;
                    else if (e->pos().x() >= geometry().width() - PIXELS_TO_ACT)
                        resizeDiagSupDer = true;
                    else
                        resizeVerSup = true;
                }
                else if (e->pos().x() <= PIXELS_TO_ACT)
                    resizeHorEsq = true;
            }
            else if (e->pos().x() >= PIXELS_TO_ACT and e->pos().x() < ui->titleBar->geometry().width()
                     and e->pos().y() >= PIXELS_TO_ACT and e->pos().y() < ui->titleBar->geometry().height())
            {
                moveWidget = true;
                dragPosition = e->globalPos() - frameGeometry().topLeft();
            }
        }

        e->accept();
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        moveWidget = false;
        allowToResize = false;
        resizeVerSup = false;
        resizeHorEsq = false;
        resizeDiagSupEsq = false;
        resizeDiagSupDer = false;

        e->accept();
    }
}

void Widget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        if (e->pos().x() < ui->tbMenu->geometry().right() and e->pos().y() < ui->tbMenu->geometry().bottom()
                and e->pos().x() >=  ui->tbMenu->geometry().x() and e->pos().y() >= ui->tbMenu->geometry().y()
                and ui->tbMenu->isVisible())
            close();
        else if (e->pos().x() < ui->titleBar->geometry().width()
                 and e->pos().y() < ui->titleBar->geometry().height()
                 and m_titleMode != FullScreenMode)
            maximizeBtnClicked();
        e->accept();
    }
}

void Widget::paintEvent (QPaintEvent *)
{
    QStyleOption opt;
    opt.init (this);
    QPainter p(this);
    style()->drawPrimitive (QStyle::PE_Widget, &opt, &p, this);
}

void Widget::moveWindow(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        if (e->buttons() & Qt::LeftButton)
        {
            move(e->globalPos() - dragPosition);
            e->accept();
        }
    }
}

void Widget::resizeWindow(QMouseEvent *e)
{
    updateFriendListWidth();
    if (!useNativeTheme)
    {
        if (allowToResize)
        {
            int xMouse = e->pos().x();
            int yMouse = e->pos().y();
            int wWidth = geometry().width();
            int wHeight = geometry().height();

            if (cursor().shape() == Qt::SizeVerCursor)
            {
                if (resizeVerSup)
                {
                    int newY = geometry().y() + yMouse;
                    int newHeight = wHeight - yMouse;

                    if (newHeight > minimumSizeHint().height())
                    {
                        resize(wWidth, newHeight);
                        move(geometry().x(), newY);
                    }
                }
                else
                    resize(wWidth, yMouse+1);
            }
            else if (cursor().shape() == Qt::SizeHorCursor)
            {
                if (resizeHorEsq)
                {
                    int newX = geometry().x() + xMouse;
                    int newWidth = wWidth - xMouse;

                    if (newWidth > minimumSizeHint().width())
                    {
                        resize(newWidth, wHeight);
                        move(newX, geometry().y());
                    }
                }
                else
                    resize(xMouse, wHeight);
            }
            else if (cursor().shape() == Qt::SizeBDiagCursor)
            {
                int newX = 0;
                int newWidth = 0;
                int newY = 0;
                int newHeight = 0;

                if (resizeDiagSupDer)
                {
                    newX = geometry().x();
                    newWidth = xMouse;
                    newY = geometry().y() + yMouse;
                    newHeight = wHeight - yMouse;
                }
                else
                {
                    newX = geometry().x() + xMouse;
                    newWidth = wWidth - xMouse;
                    newY = geometry().y();
                    newHeight = yMouse;
                }

                if (newWidth >= minimumSizeHint().width() and newHeight >= minimumSizeHint().height())
                {
                    resize(newWidth, newHeight);
                    move(newX, newY);
                }
                else if (newWidth >= minimumSizeHint().width())
                {
                    resize(newWidth, wHeight);
                    move(newX, geometry().y());
                }
                else if (newHeight >= minimumSizeHint().height())
                {
                    resize(wWidth, newHeight);
                    move(geometry().x(), newY);
                }
            }
            else if (cursor().shape() == Qt::SizeFDiagCursor)
            {
                if (resizeDiagSupEsq)
                {
                    int newX = geometry().x() + xMouse;
                    int newWidth = wWidth - xMouse;
                    int newY = geometry().y() + yMouse;
                    int newHeight = wHeight - yMouse;

                    if (newWidth >= minimumSizeHint().width() and newHeight >= minimumSizeHint().height())
                    {
                        resize(newWidth, newHeight);
                        move(newX, newY);
                    }
                    else if (newWidth >= minimumSizeHint().width())
                    {
                        resize(newWidth, wHeight);
                        move(newX, geometry().y());
                    }
                    else if (newHeight >= minimumSizeHint().height())
                    {
                        resize(wWidth, newHeight);
                        move(geometry().x(), newY);
                    }
                }
                else
                    resize(xMouse+1, yMouse+1);
            }

            e->accept();
        }
    }
}

void Widget::setCentralWidget(QWidget *widget, const QString &widgetName)
{
    connect(widget, SIGNAL(cancelled()), this, SLOT(close()));

    centralLayout->addWidget(widget);
    //ui->centralWidget->setLayout(centralLayout);
    ui->LTitle->setText(widgetName);
}

void Widget::setTitlebarMode(const TitleMode &flag)
{
    m_titleMode = flag;

    switch (m_titleMode)
    {
        case CleanTitle:
            ui->tbMenu->setHidden(true);
            ui->pbMin->setHidden(true);
            ui->pbMax->setHidden(true);
            ui->pbClose->setHidden(true);
            break;
        case OnlyCloseButton:
            ui->tbMenu->setHidden(true);
            ui->pbMin->setHidden(true);
            ui->pbMax->setHidden(true);
            break;
        case MenuOff:
            ui->tbMenu->setHidden(true);
            break;
        case MaxMinOff:
            ui->pbMin->setHidden(true);
            ui->pbMax->setHidden(true);
            break;
        case FullScreenMode:
            ui->pbMax->setHidden(true);
            showMaximized();
            break;
        case MaximizeModeOff:
            ui->pbMax->setHidden(true);
            break;
        case MinimizeModeOff:
            ui->pbMin->setHidden(true);
            break;
        case FullTitle:
            ui->tbMenu->setVisible(true);
            ui->pbMin->setVisible(true);
            ui->pbMax->setVisible(true);
            ui->pbClose->setVisible(true);
            break;
            break;
        default:
            ui->tbMenu->setVisible(true);
            ui->pbMin->setVisible(true);
            ui->pbMax->setVisible(true);
            ui->pbClose->setVisible(true);
            break;
    }
    ui->LTitle->setVisible(true);
}

void Widget::setTitlebarMenu(QMenu *menu, const QString &icon)
{
    ui->tbMenu->setMenu(menu);
    ui->tbMenu->setIcon(QIcon(icon));
}

void Widget::maximizeBtnClicked()
{
    if (isFullScreen() or isMaximized())
    {
        ui->pbMax->setIcon(QIcon(":/ui/images/app_max.png"));
        setWindowState(windowState() & ~Qt::WindowFullScreen & ~Qt::WindowMaximized);
    }
    else
    {
        ui->pbMax->setIcon(QIcon(":/ui/images/app_rest.png"));
        setWindowState(windowState() | Qt::WindowFullScreen | Qt::WindowMaximized);
    }
}

void Widget::minimizeBtnClicked()
{
    if (isMinimized())
    {
        setWindowState(windowState() & ~Qt::WindowMinimized);
    }
    else
    {
        setWindowState(windowState() | Qt::WindowMinimized);
    }
}

void Widget::setStatusOnline()
{
    core->setStatus(Status::Online);
}

void Widget::setStatusAway()
{
    core->setStatus(Status::Away);
}

void Widget::setStatusBusy()
{
    core->setStatus(Status::Busy);
}

bool Widget::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::Wheel)
    {
        QWheelEvent * whlEvnt =  static_cast< QWheelEvent * >( event );
        whlEvnt->angleDelta().setX(0);
    }
    return false;
}
