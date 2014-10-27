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
#include "ui_mainwindow.h"
#include "src/core.h"
#include "src/misc/settings.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "tool/friendrequestdialog.h"
#include "friendwidget.h"
#include "src/grouplist.h"
#include "src/group.h"
#include "groupwidget.h"
#include "form/groupchatform.h"
#include "src/misc/style.h"
#include "friendlistwidget.h"
#include "src/video/camera.h"
#include "form/chatform.h"
#include "maskablepixmapwidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QBuffer>
#include <QPainter>
#include <QMouseEvent>
#include <QClipboard>
#include <QThread>
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>
#include <QStyleFactory>
#include <QTranslator>
#include "src/historykeeper.h"
#include <tox/tox.h>
#include "form/inputpassworddialog.h"

Widget *Widget::instance{nullptr};

Widget::Widget(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      activeChatroomWidget{nullptr}
{   
    translator = new QTranslator;
    setTranslation();
}

void Widget::init()
{
    
    ui->setupUi(this);
    
    if (QSystemTrayIcon::isSystemTrayAvailable() == true)
    {
        icon = new QSystemTrayIcon(this);
        icon->setIcon(this->windowIcon());
        trayMenu = new QMenu;
        
        statusOnline = new QAction(tr("online"), this);
        statusOnline->setIcon(QIcon(":ui/statusButton/dot_online.png"));
        connect(statusOnline, SIGNAL(triggered()), this, SLOT(setStatusOnline()));
        statusAway = new QAction(tr("away"), this);
        statusAway->setIcon(QIcon(":ui/statusButton/dot_idle.png"));
        connect(statusAway, SIGNAL(triggered()), this, SLOT(setStatusAway()));
        statusBusy = new QAction(tr("busy"), this);
        connect(statusBusy, SIGNAL(triggered()), this, SLOT(setStatusBusy()));
        statusBusy->setIcon(QIcon(":ui/statusButton/dot_busy.png"));
        actionQuit = new QAction(tr("&Quit"), this);
        connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
        
        trayMenu->addAction(new QAction(tr("Change status to:"), this));
        trayMenu->addAction(statusOnline);
        trayMenu->addAction(statusAway);
        trayMenu->addAction(statusBusy);
        trayMenu->addSeparator();
        trayMenu->addAction(actionQuit);
        icon->setContextMenu(trayMenu);
        
        connect(icon,
                SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this,
                SLOT(onIconClick(QSystemTrayIcon::ActivationReason)));
        
        icon->show();
        
        if(Settings::getInstance().getAutostartInTray() == false)
            this->show();
    }
    else
    {
        qWarning() << "No system tray detected!";
        this->show();
    }

    ui->statusbar->hide();
    ui->menubar->hide();

    //restore window state
    restoreGeometry(Settings::getInstance().getWindowGeometry());
    restoreState(Settings::getInstance().getWindowState());
    ui->mainSplitter->restoreState(Settings::getInstance().getSplitterState());

    layout()->setContentsMargins(0, 0, 0, 0);
    ui->friendList->setStyleSheet(Style::getStylesheet(":ui/friendList/friendList.css"));

    profilePicture = new MaskablePixmapWidget(this, QSize(40, 40), ":/img/avatar_mask.png");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.png"));
    profilePicture->setClickable(true);
    ui->horizontalLayout_3->insertWidget(0, profilePicture);
    ui->horizontalLayout_3->insertSpacing(1, 7);

    ui->mainContent->setLayout(new QVBoxLayout());
    ui->mainHead->setLayout(new QVBoxLayout());
    ui->mainHead->layout()->setMargin(0);
    ui->mainHead->layout()->setSpacing(0);
    

    if(QStyleFactory::keys().contains(Settings::getInstance().getStyle())
            && Settings::getInstance().getStyle() != "None")
    {
        ui->mainHead->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
        ui->mainContent->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
    }
    
    ui->mainHead->setStyleSheet(Style::getStylesheet(":ui/settings/mainHead.css"));    
    ui->mainContent->setStyleSheet(Style::getStylesheet(":ui/settings/mainContent.css"));

    contactListWidget = new FriendListWidget();
    ui->friendList->setWidget(contactListWidget);
    ui->friendList->setLayoutDirection(Qt::RightToLeft);

    ui->nameLabel->setEditable(true);
    ui->statusLabel->setEditable(true);

    ui->statusPanel->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));

    QMenu *statusButtonMenu = new QMenu(ui->statusButton);
    QAction* setStatusOnline = statusButtonMenu->addAction(Widget::tr("Online","Button to set your status to 'Online'"));
    setStatusOnline->setIcon(QIcon(":ui/statusButton/dot_online.png"));
    QAction* setStatusAway = statusButtonMenu->addAction(Widget::tr("Away","Button to set your status to 'Away'"));
    setStatusAway->setIcon(QIcon(":ui/statusButton/dot_idle.png"));
    QAction* setStatusBusy = statusButtonMenu->addAction(Widget::tr("Busy","Button to set your status to 'Busy'"));
    setStatusBusy->setIcon(QIcon(":ui/statusButton/dot_busy.png"));
    ui->statusButton->setMenu(statusButtonMenu);

    // disable proportional scaling
    ui->mainSplitter->setStretchFactor(0,0);
    ui->mainSplitter->setStretchFactor(1,1);

    ui->statusButton->setProperty("status", "offline");
    Style::repolish(ui->statusButton);

    // Disable some widgets until we're connected to the DHT
    ui->statusButton->setEnabled(false);

    idleTimer = new QTimer();
    int mins = Settings::getInstance().getAutoAwayTime();
    if (mins > 0)
        idleTimer->start(mins * 1000*60);

    qRegisterMetaType<Status>("Status");
    qRegisterMetaType<vpx_image>("vpx_image");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<QPixmap>("QPixmap");
    qRegisterMetaType<ToxFile>("ToxFile");
    qRegisterMetaType<ToxFile::FileDirection>("ToxFile::FileDirection");
    qRegisterMetaType<Core::PasswordType>("Core::PasswordType");

    QString profilePath = detectProfile();
    coreThread = new QThread(this);
    core = new Core(Camera::getInstance(), coreThread, profilePath);
    core->moveToThread(coreThread);
    connect(coreThread, &QThread::started, core, &Core::start);
    
    filesForm = new FilesForm();
    addFriendForm = new AddFriendForm;
    settingsWidget = new SettingsWidget();

    connect(core, &Core::connected, this, &Widget::onConnected);
    connect(core, &Core::disconnected, this, &Widget::onDisconnected);
    connect(core, &Core::failedToStart, this, &Widget::onFailedToStartCore);
    connect(core, &Core::badProxy, this, &Widget::onBadProxyCore);
    connect(core, &Core::statusSet, this, &Widget::onStatusSet);
    connect(core, &Core::usernameSet, this, &Widget::setUsername);
    connect(core, &Core::statusMessageSet, this, &Widget::setStatusMessage);
    connect(core, &Core::selfAvatarChanged, this, &Widget::onSelfAvatarLoaded);
    connect(core, SIGNAL(fileDownloadFinished(const QString&)), filesForm, SLOT(onFileDownloadComplete(const QString&)));
    connect(core, SIGNAL(fileUploadFinished(const QString&)), filesForm, SLOT(onFileUploadComplete(const QString&)));
    connect(core, &Core::friendAdded, this, &Widget::addFriend);
    connect(core, &Core::failedToAddFriend, this, &Widget::addFriendFailed);
    connect(core, &Core::friendUsernameChanged, this, &Widget::onFriendUsernameChanged);
    connect(core, &Core::friendStatusChanged, this, &Widget::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, this, &Widget::onFriendStatusMessageChanged);
    connect(core, &Core::friendRequestReceived, this, &Widget::onFriendRequestReceived);
    connect(core, &Core::friendMessageReceived, this, &Widget::onFriendMessageReceived);
    connect(core, &Core::groupInviteReceived, this, &Widget::onGroupInviteReceived);
    connect(core, &Core::groupMessageReceived, this, &Widget::onGroupMessageReceived);
    connect(core, &Core::groupNamelistChanged, this, &Widget::onGroupNamelistChanged);
    connect(core, &Core::emptyGroupCreated, this, &Widget::onEmptyGroupCreated);
    connect(core, &Core::avInvite, this, &Widget::playRingtone);
    connect(core, &Core::blockingClearContacts, this, &Widget::clearContactsList, Qt::BlockingQueuedConnection);
    connect(core, &Core::blockingGetPassword, this, &Widget::getPassword, Qt::BlockingQueuedConnection);

    connect(core, SIGNAL(messageSentResult(int,QString,int)), this, SLOT(onMessageSendResult(int,QString,int)));
    connect(core, SIGNAL(groupSentResult(int,QString,int)), this, SLOT(onGroupSendResult(int,QString,int)));

    connect(this, &Widget::statusSet, core, &Core::setStatus);
    connect(this, &Widget::friendRequested, core, &Core::requestFriendship);
    connect(this, &Widget::friendRequestAccepted, core, &Core::acceptFriendRequest);
    connect(this, &Widget::changeProfile, core, &Core::switchConfiguration);

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(onAddClicked()));
    connect(ui->groupButton, SIGNAL(clicked()), this, SLOT(onGroupClicked()));
    connect(ui->transferButton, SIGNAL(clicked()), this, SLOT(onTransferClicked()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onSettingsClicked()));
    connect(ui->nameLabel, SIGNAL(textChanged(QString, QString)), this, SLOT(onUsernameChanged(QString, QString)));
    connect(ui->statusLabel, SIGNAL(textChanged(QString, QString)), this, SLOT(onStatusMessageChanged(QString, QString)));
    connect(profilePicture, SIGNAL(clicked()), this, SLOT(onAvatarClicked()));
    connect(setStatusOnline, SIGNAL(triggered()), this, SLOT(setStatusOnline()));
    connect(setStatusAway, SIGNAL(triggered()), this, SLOT(setStatusAway()));
    connect(setStatusBusy, SIGNAL(triggered()), this, SLOT(setStatusBusy()));
    connect(addFriendForm, SIGNAL(friendRequested(QString, QString)), this, SIGNAL(friendRequested(QString, QString)));
    connect(idleTimer, &QTimer::timeout, this, &Widget::onUserAway);

    coreThread->start();

    addFriendForm->show(*ui);
}

void Widget::setTranslation()
{
    // Load translations
    QCoreApplication::removeTranslator(translator);
    QString locale;
    if ((locale = Settings::getInstance().getTranslation()).isEmpty())
        locale = QLocale::system().name().section('_', 0, 0);
    
    if (translator->load(locale, ":translations/"))
        qDebug() << "Loaded translation" << locale;
    else
        qDebug() << "Error loading translation" << locale;
    QCoreApplication::installTranslator(translator);
}

Widget::~Widget()
{
    core->saveConfiguration();
    coreThread->exit();
    coreThread->wait(500); // In case of deadlock (can happen with QtAudio/PA bugs)
    if (!coreThread->isFinished())
        coreThread->terminate();
    delete core;
    delete settingsWidget;
    delete addFriendForm;
    delete filesForm;

    for (Friend* f : FriendList::friendList)
        delete f;
    FriendList::friendList.clear();
    for (Group* g : GroupList::groupList)
        delete g;
    GroupList::groupList.clear();
    delete statusAway;
    delete statusBusy;
    delete statusOnline;
    delete actionQuit;
    delete trayMenu;
    delete icon;
    delete ui;
    delete translator;
    instance = nullptr;
}

Widget* Widget::getInstance()
{
    
    if (!instance)
    {
        instance = new Widget();
        instance->init();
    }
    return instance;
}

QThread* Widget::getCoreThread()
{
    return coreThread;
}

void Widget::closeEvent(QCloseEvent *event)
{
    if(Settings::getInstance().getCloseToTray() == true)
    {
        event->ignore();
        this->hide();
    }
    else
    {
        Settings::getInstance().setWindowGeometry(saveGeometry());
        Settings::getInstance().setWindowState(saveState());
        Settings::getInstance().setSplitterState(ui->mainSplitter->saveState());
        QWidget::closeEvent(event);
    }
}

void Widget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if(isMinimized() == true
                && Settings::getInstance().getMinimizeToTray() == true)
        {
            this->hide();
        }
    }
}

QString Widget::detectProfile()
{
    QDir dir(Settings::getSettingsDirPath());
    QString path, profile = Settings::getInstance().getCurrentProfile();
    path = dir.filePath(profile + Core::TOX_EXT);
    QFile file(path);
    if (profile == "" || !file.exists())
    {
        Settings::getInstance().setCurrentProfile("");
#if 1 // deprecation attempt
        // if the last profile doesn't exist, fall back to old "data"
        path = dir.filePath(Core::CONFIG_FILE_NAME);
        QFile file(path);
        if (file.exists())
            return path;
        else if (QFile(path = dir.filePath("tox_save")).exists()) // also import tox_save if no data
            return path;
        else
#endif
        {
            profile = askProfiles();
            if (profile != "")
                return dir.filePath(profile + Core::TOX_EXT);
            else
                return "";
        }
    }
    else
        return path;
}

QList<QString> Widget::searchProfiles()
{
    QList<QString> out;
    QDir dir(Settings::getSettingsDirPath());
	dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	dir.setNameFilters(QStringList("*.tox"));
	for(QFileInfo file : dir.entryInfoList())
		out += file.completeBaseName();
	return out;
}

QString Widget::askProfiles()
{   // TODO: allow user to create new Tox ID, even if a profile already exists
    QList<QString> profiles = searchProfiles();
    if (profiles.empty()) return "";
    bool ok;
    QString profile = QInputDialog::getItem(this, 
                                            tr("Choose a profile"),
                                            tr("Please choose which identity to use"),
                                            profiles,
                                            0, // which slot to start on
                                            false, // if the user can enter their own input
                                            &ok);
    if (!ok) // user cancelled
    {
        qApp->quit();
        return "";
    }
    else
        return profile;
}

void Widget::setIdleTimer(int minutes)
{
    idleTimer->start(minutes * 1000*60);
}

QString Widget::getUsername()
{
    return core->getUsername();
}

void Widget::onAvatarClicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose a profile picture"), QDir::homePath());
    if (filename.isEmpty())
        return;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
    {
        QMessageBox::critical(this, tr("Error"), tr("Unable to open this file"));
        return;
    }

    QPixmap pic;
    if (!pic.loadFromData(file.readAll()))
    {
        QMessageBox::critical(this, tr("Error"), tr("Unable to read this image"));
        return;
    }

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pic.save(&buffer, "PNG");
    buffer.close();

    if (bytes.size() >= TOX_AVATAR_MAX_DATA_LENGTH)
    {
        pic = pic.scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        bytes.clear();
        buffer.open(QIODevice::WriteOnly);
        pic.save(&buffer, "PNG");
        buffer.close();
    }

    if (bytes.size() >= TOX_AVATAR_MAX_DATA_LENGTH)
    {
        QMessageBox::critical(this, tr("Error"), tr("This image is too big"));
        return;
    }

    core->setAvatar(TOX_AVATAR_FORMAT_PNG, bytes);
}

void Widget::onSelfAvatarLoaded(const QPixmap& pic)
{
    profilePicture->setPixmap(pic);
}

void Widget::onConnected()
{
    ui->statusButton->setEnabled(true);
    emit statusSet(Status::Online);
}

void Widget::onDisconnected()
{
    ui->statusButton->setEnabled(false);
    emit statusSet(Status::Offline);
}

void Widget::onFailedToStartCore()
{
    QMessageBox critical(this);
    critical.setText(tr("Toxcore failed to start, the application will terminate after you close this message."));
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    qApp->quit();
}

void Widget::onBadProxyCore()
{
    QMessageBox critical(this);
    critical.setText(tr("toxcore failed to start with your proxy settings. qTox cannot run; please modify your "
               "settings and restart.", "popup text"));
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    onSettingsClicked();
}

void Widget::onStatusSet(Status status)
{
    //We have to use stylesheets here, there's no way to
    //prevent the button icon from moving when pressed otherwise
    switch (status)
    {
    case Status::Online:
        ui->statusButton->setProperty("status" ,"online");
        break;
    case Status::Away:
        ui->statusButton->setProperty("status" ,"away");
        break;
    case Status::Busy:
        ui->statusButton->setProperty("status" ,"busy");
        break;
    case Status::Offline:
        ui->statusButton->setProperty("status" ,"offline");
        break;
    }

    Style::repolish(ui->statusButton);
}

void Widget::onAddClicked()
{
    hideMainForms();
    addFriendForm->show(*ui);
}

void Widget::onGroupClicked()
{
    core->createGroup();
}

void Widget::onTransferClicked()
{
    hideMainForms();
    filesForm->show(*ui);
    activeChatroomWidget = nullptr;
}

void Widget::onIconClick(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        if(this->isHidden() == true)
            this->show();
        else
            this->hide();
        case QSystemTrayIcon::DoubleClick:    
            break;
        case QSystemTrayIcon::MiddleClick:
            break;
        default:
            ;
    }
}

void Widget::onSettingsClicked()
{
    hideMainForms();
    settingsWidget->show(*ui);
    activeChatroomWidget = nullptr;
}

void Widget::hideMainForms()
{
    QLayoutItem* item;
    while ((item = ui->mainHead->layout()->takeAt(0)) != 0)
        item->widget()->hide();
    while ((item = ui->mainContent->layout()->takeAt(0)) != 0)
        item->widget()->hide();

    if (activeChatroomWidget != nullptr)
    {
        activeChatroomWidget->setAsInactiveChatroom();
    }
}

void Widget::onUsernameChanged(const QString& newUsername, const QString& oldUsername)
{
    ui->nameLabel->setText(oldUsername); // restore old username until Core tells us to set it
    ui->nameLabel->setToolTip(oldUsername); // for overlength names
    core->setUsername(newUsername);
}

void Widget::setUsername(const QString& username)
{
    ui->nameLabel->setText(username);
    ui->nameLabel->setToolTip(username); // for overlength names
}

void Widget::onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage)
{
    ui->statusLabel->setText(oldStatusMessage); // restore old status message until Core tells us to set it
    ui->statusLabel->setToolTip(oldStatusMessage); // for overlength messsages
    core->setStatusMessage(newStatusMessage);
}

void Widget::setStatusMessage(const QString &statusMessage)
{
    ui->statusLabel->setText(statusMessage);
    ui->statusLabel->setToolTip(statusMessage); // for overlength messsages
}

void Widget::addFriend(int friendId, const QString &userId)
{
    qDebug() << "Widget: Adding friend with id" << userId;
    Friend* newfriend = FriendList::addFriend(friendId, userId);
    QLayout* layout = contactListWidget->getFriendLayout(Status::Offline);
    layout->addWidget(newfriend->widget);
    connect(newfriend->widget, SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), this, SLOT(onChatroomWidgetClicked(GenericChatroomWidget*)));
    connect(newfriend->widget, SIGNAL(removeFriend(int)), this, SLOT(removeFriend(int)));
    connect(newfriend->widget, SIGNAL(copyFriendIdToClipboard(int)), this, SLOT(copyFriendIdToClipboard(int)));
    connect(newfriend->widget, SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), newfriend->chatForm, SLOT(focusInput()));
    connect(newfriend->chatForm, SIGNAL(sendMessage(int,QString)), core, SLOT(sendMessage(int,QString)));
    connect(newfriend->chatForm, &GenericChatForm::sendAction, core, &Core::sendAction);
    connect(newfriend->chatForm, SIGNAL(sendFile(int32_t, QString, QString, long long)), core, SLOT(sendFile(int32_t, QString, QString, long long)));
    connect(newfriend->chatForm, SIGNAL(answerCall(int)), core, SLOT(answerCall(int)));
    connect(newfriend->chatForm, SIGNAL(hangupCall(int)), core, SLOT(hangupCall(int)));
    connect(newfriend->chatForm, SIGNAL(startCall(int)), core, SLOT(startCall(int)));
    connect(newfriend->chatForm, SIGNAL(startVideoCall(int,bool)), core, SLOT(startCall(int,bool)));
    connect(newfriend->chatForm, SIGNAL(cancelCall(int,int)), core, SLOT(cancelCall(int,int)));
    connect(newfriend->chatForm, SIGNAL(micMuteToggle(int)), core, SLOT(micMuteToggle(int)));
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
    connect(core, &Core::avMediaChange, newfriend->chatForm, &ChatForm::onAvMediaChange);
    connect(core, &Core::avCallFailed, newfriend->chatForm, &ChatForm::onAvCallFailed);
    connect(core, &Core::friendAvatarChanged, newfriend->chatForm, &ChatForm::onAvatarChange);
    connect(core, &Core::friendAvatarChanged, newfriend->widget, &FriendWidget::onAvatarChange);
    connect(core, &Core::friendAvatarRemoved, newfriend->chatForm, &ChatForm::onAvatarRemoved);
    connect(core, &Core::friendAvatarRemoved, newfriend->widget, &FriendWidget::onAvatarRemoved);

    // Try to get the avatar from the cache
    QPixmap avatar = Settings::getInstance().getSavedAvatar(userId);
    if (!avatar.isNull())
    {
        qWarning() << "Widget: loadded avatar for id" << userId;
        newfriend->chatForm->onAvatarChange(friendId, avatar);
        newfriend->widget->onAvatarChange(friendId, avatar);
    }
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

    contactListWidget->moveWidget(f->widget, status);

    f->friendStatus = status;
    f->widget->updateStatusLight();
    
    //won't print the message if there were no messages before
    if(f->chatForm->getNumberOfMessages() != 0
            && Settings::getInstance().getStatusChangeNotificationEnabled() == true)
    {
        QString fStatus = "";
        switch(f->friendStatus){
        case Status::Away:
            fStatus = tr("away", "contact status"); break;
        case Status::Busy:
            fStatus = tr("busy", "contact status"); break;
        case Status::Offline:
            fStatus = tr("offline", "contact status"); break;
        default:
            fStatus = tr("online", "contact status"); break;
        }
        f->chatForm->addSystemInfoMessage(tr("%1 is now %2", "e.g. \"Dubslow is now online\"").arg(f->getName()).arg(fStatus),
                                          "white", QDateTime::currentDateTime());
    }
}

void Widget::onFriendStatusMessageChanged(int friendId, const QString& message)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    QString str = message; str.replace('\n', ' ');
    str.remove('\r'); str.remove(QChar((char)0)); // null terminator...
    f->setStatusMessage(str);
}

void Widget::onFriendUsernameChanged(int friendId, const QString& username)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    QString str = username; str.replace('\n', ' ');
    str.remove('\r'); str.remove(QChar((char)0)); // null terminator...
    f->setName(str);
}

void Widget::onChatroomWidgetClicked(GenericChatroomWidget *widget)
{
    hideMainForms();
    widget->setChatForm(*ui);
    if (activeChatroomWidget != nullptr)
    {
        activeChatroomWidget->setAsInactiveChatroom();
    }
    activeChatroomWidget = widget;
    widget->setAsActiveChatroom();
    widget->resetEventFlags();
    widget->updateStatusLight();
}

void Widget::onFriendMessageReceived(int friendId, const QString& message, bool isAction)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    QDateTime timestamp = QDateTime::currentDateTime();
    f->chatForm->addMessage(f->getName(), message, isAction, timestamp);

    if (isAction)
        HistoryKeeper::getInstance()->addChatEntry(f->userId, "/me " + message, f->userId, timestamp);
    else
        HistoryKeeper::getInstance()->addChatEntry(f->userId, message, f->userId, timestamp);

    if (activeChatroomWidget != nullptr)
    {
        if ((static_cast<GenericChatroomWidget*>(f->widget) != activeChatroomWidget) || isMinimized() || !isActiveWindow())
        {
            f->hasNewEvents = 1;
            newMessageAlert();
        }
    }
    else
    {
        f->hasNewEvents = 1;
        newMessageAlert();
    }

    f->widget->updateStatusLight();
}

void Widget::newMessageAlert()
{
    QApplication::alert(this);

    static QFile sndFile(":audio/notification.pcm");
    static QByteArray sndData;
    if (sndData.isEmpty())
    {
        sndFile.open(QIODevice::ReadOnly);
        sndData = sndFile.readAll();
        sndFile.close();
    }

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, sndData.data(), sndData.size(), 44100);
    alSourcei(core->alMainSource, AL_BUFFER, buffer);
    alSourcePlay(core->alMainSource);
}

void Widget::playRingtone()
{
    QApplication::alert(this);

    static QFile sndFile1(":audio/ToxicIncomingCall.pcm"); // for whatever reason this plays slower/downshifted from what any other program plays the file as... but whatever
    static QByteArray sndData1;
    if (sndData1.isEmpty())
    {
        sndFile1.open(QIODevice::ReadOnly);
        sndData1 = sndFile1.readAll();
        sndFile1.close();
    }

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, sndData1.data(), sndData1.size(), 44100);
    alSourcei(core->alMainSource, AL_BUFFER, buffer);
    alSourcePlay(core->alMainSource);
}

void Widget::onFriendRequestReceived(const QString& userId, const QString& message)
{
    FriendRequestDialog dialog(this, userId, message);

    if (dialog.exec() == QDialog::Accepted)
        emit friendRequestAccepted(userId);
}

void Widget::removeFriend(Friend* f)
{
    f->widget->setAsInactiveChatroom();
    if (static_cast<GenericChatroomWidget*>(f->widget) == activeChatroomWidget)
        activeChatroomWidget = nullptr;
    FriendList::removeFriend(f->friendId);
    core->removeFriend(f->friendId);
    delete f;
    if (ui->mainHead->layout()->isEmpty())
        onAddClicked();
}

void Widget::removeFriend(int friendId)
{
    removeFriend(FriendList::findFriend(friendId));
}

void Widget::clearContactsList()
{
    for (Friend* f : FriendList::friendList)
        removeFriend(f);
    for (Group* g : GroupList::groupList)
        removeGroup(g);
}

void Widget::copyFriendIdToClipboard(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    if (f != nullptr)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(core->getFriendAddress(f->friendId), QClipboard::Clipboard);
    }
}

void Widget::onGroupInviteReceived(int32_t friendId, const uint8_t* publicKey,uint16_t length)
{
    int groupId = core->joinGroupchat(friendId, publicKey,length);
    if (groupId == -1)
    {
        qWarning() << "Widget::onGroupInviteReceived: Unable to accept invitation";
        return;
    }
}

void Widget::onGroupMessageReceived(int groupnumber, const QString& message, const QString& author)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    QString name = core->getUsername();
    bool targeted = (author != name) && message.contains(name, Qt::CaseInsensitive);
    if (targeted)
        g->chatForm->addAlertMessage(author, message, QDateTime::currentDateTime());
    else
        g->chatForm->addMessage(author, message, false, QDateTime::currentDateTime());

    if ((static_cast<GenericChatroomWidget*>(g->widget) != activeChatroomWidget) || isMinimized() || !isActiveWindow())
    {
        g->hasNewMessages = 1;
        if (targeted)
        {
            newMessageAlert();
            g->userWasMentioned = 1; // useful for highlighting line or desktop notifications
        }
        g->widget->updateStatusLight();
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

    QString name = core->getGroupPeerName(groupnumber, peernumber);
    TOX_CHAT_CHANGE change = static_cast<TOX_CHAT_CHANGE>(Change);
    if (change == TOX_CHAT_CHANGE_PEER_ADD)
    {
        if (name.isEmpty())
            name = tr("<Unknown>", "Placeholder when we don't know someone's name in a group chat");
        g->addPeer(peernumber,name);
        //g->chatForm->addSystemInfoMessage(tr("%1 has joined the chat").arg(name), "green");
        // we can't display these messages until irungentoo fixes peernumbers
        // https://github.com/irungentoo/toxcore/issues/1128
    }
    else if (change == TOX_CHAT_CHANGE_PEER_DEL)
    {
        g->removePeer(peernumber);
        //g->chatForm->addSystemInfoMessage(tr("%1 has left the chat").arg(name), "silver");
    }
    else if (change == TOX_CHAT_CHANGE_PEER_NAME) // core overwrites old name before telling us it changed...
        g->updatePeer(peernumber,core->getGroupPeerName(groupnumber, peernumber));
}

void Widget::removeGroup(Group* g)
{
    g->widget->setAsInactiveChatroom();
    if (static_cast<GenericChatroomWidget*>(g->widget) == activeChatroomWidget)
        activeChatroomWidget = nullptr;
    GroupList::removeGroup(g->groupId);
    core->removeGroup(g->groupId);
    delete g;
    if (ui->mainHead->layout()->isEmpty())
        onAddClicked();
}

void Widget::removeGroup(int groupId)
{
    removeGroup(GroupList::findGroup(groupId));
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
    QLayout* layout = contactListWidget->getGroupLayout();
    layout->addWidget(newgroup->widget);
    newgroup->widget->updateStatusLight();

    connect(newgroup->widget, SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), this, SLOT(onChatroomWidgetClicked(GenericChatroomWidget*)));
    connect(newgroup->widget, SIGNAL(removeGroup(int)), this, SLOT(removeGroup(int)));
    connect(newgroup->widget, SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), newgroup->chatForm, SLOT(focusInput()));
    connect(newgroup->chatForm, SIGNAL(sendMessage(int,QString)), core, SLOT(sendGroupMessage(int,QString)));
    return newgroup;
}

void Widget::onEmptyGroupCreated(int groupId)
{
    createGroup(groupId);
}

bool Widget::isFriendWidgetCurActiveWidget(Friend* f)
{
    if (!f)
        return false;

    if (activeChatroomWidget == static_cast<GenericChatroomWidget*>(f->widget))
        return true;
    else
        return false;
}

bool Widget::event(QEvent * e)
{
    switch(e->type()) {
        case QEvent::WindowActivate:
            if (activeChatroomWidget != nullptr)
            {
                activeChatroomWidget->resetEventFlags();
                activeChatroomWidget->updateStatusLight();
            }
        // http://qt-project.org/faq/answer/how_can_i_detect_a_period_of_no_user_interaction
        // Detecting global inactivity, like Skype, is possible but not via Qt:
        // http://stackoverflow.com/a/21905027/1497645
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::Wheel:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
            if (autoAwayActive)
            {
                qDebug() << "Widget: auto away deactivated";
                autoAwayActive = false;
                emit statusSet(Status::Online);
                int mins = Settings::getInstance().getAutoAwayTime();
                if (mins > 0)
                    idleTimer->start(mins * 1000*60);
            }
        default:
            break;
    }

    return QWidget::event(e);
}

void Widget::onUserAway()
{
    if (Settings::getInstance().getAutoAwayTime() > 0
        && ui->statusButton->property("status").toString() == "online") // leave user-set statuses in place
    {
        qDebug() << "Widget: auto away activated";
        emit statusSet(Status::Away);
        autoAwayActive = true;
    }
    idleTimer->stop();
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

void Widget::onMessageSendResult(int friendId, const QString& message, int messageId)
{
    Q_UNUSED(message)
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    if (!messageId)
        f->chatForm->addSystemInfoMessage("Message failed to send", "red", QDateTime::currentDateTime());
}

void Widget::onGroupSendResult(int groupId, const QString& message, int result)
{
    Q_UNUSED(message)
    Group* g = GroupList::findGroup(groupId);
    if (!g)
        return;

    if (result == -1)
        g->chatForm->addSystemInfoMessage("Message failed to send", "red", QDateTime::currentDateTime());
}

void Widget::getPassword(QString info, int passtype, uint8_t* salt)
{
    Core::PasswordType pt = static_cast<Core::PasswordType>(passtype);
    InputPasswordDialog dialog(info);
    if (dialog.exec())
    {
        QString pswd = dialog.getPassword();
        if (pswd.isEmpty())
            core->clearPassword(pt);
        else
            core->setPassword(pswd, pt, salt);
    }
}

QMessageBox::StandardButton Widget::showWarningMsgBox(const QString& title, const QString& msg, QMessageBox::StandardButtons buttons)
{
    // We can only display widgets from the GUI thread
    if (QThread::currentThread() != qApp->thread())
    {
        QMessageBox::StandardButton ret;
        QMetaObject::invokeMethod(this, "showWarningMsgBox", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(QMessageBox::StandardButton, ret),
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg),
                                  Q_ARG(QMessageBox::StandardButtons, buttons));
        return ret;
    }
    else
    {
        return QMessageBox::warning(this, title, msg, buttons);
    }
}
