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
#include "src/historykeeper.h"
#include "src/autoupdate.h"
#include "src/audio.h"
#include "src/platform/timer.h"
#include "systemtrayicon.h"
#include "src/nexus.h"
#include "src/offlinemsgengine.h"
#include <cassert>
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
#include <QDialogButtonBox>
#include <QTimer>
#include <QStyleFactory>
#include <QTranslator>
#include <tox/tox.h>

#ifdef Q_OS_ANDROID
#define IS_ON_DESKTOP_GUI 0
#else
#define IS_ON_DESKTOP_GUI 1
#endif

void toxActivateEventHandler(const QByteArray& data)
{
    if (data != "$activate")
        return;
    Widget::getInstance()->forceShow();
}

Widget *Widget::instance{nullptr};

Widget::Widget(QWidget *parent)
    : QMainWindow(parent),
      icon{nullptr},
      ui(new Ui::MainWindow),
      activeChatroomWidget{nullptr},
      eventFlag(false),
      eventIcon(false)
{   
    translator = new QTranslator;
    setTranslation();
}

void Widget::init()
{
    ui->setupUi(this);

    timer = new QTimer();
    timer->start(1000);
    offlineMsgTimer = new QTimer();
    offlineMsgTimer->start(15000);

    //restore window state
    restoreGeometry(Settings::getInstance().getWindowGeometry());
    restoreState(Settings::getInstance().getWindowState());
    ui->mainSplitter->restoreState(Settings::getInstance().getSplitterState());

    statusOnline = new QAction(tr("Online", "Button to set your status to 'Online'"), this);
    statusOnline->setIcon(QIcon(":img/status/dot_online.png"));
    connect(statusOnline, SIGNAL(triggered()), this, SLOT(setStatusOnline()));
    statusAway = new QAction(tr("Away", "Button to set your status to 'Away'"), this);
    statusAway->setIcon(QIcon(":img/status/dot_idle.png"));
    connect(statusAway, SIGNAL(triggered()), this, SLOT(setStatusAway()));
    statusBusy = new QAction(tr("Busy", "Button to set your status to 'Busy'"), this);
    statusBusy->setIcon(QIcon(":img/status/dot_busy.png"));
    connect(statusBusy, SIGNAL(triggered()), this, SLOT(setStatusBusy()));

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        icon = new SystemTrayIcon;
        updateTrayIcon();
        trayMenu = new QMenu;

        actionQuit = new QAction(tr("&Quit"), this);
        connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

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
        icon->hide();

        if (Settings::getInstance().getShowSystemTray())
        {
            icon->show();
            if (Settings::getInstance().getAutostartInTray() == false)
                this->show();
        }
        else
            this->show();
    }
    else
    {
        qWarning() << "Widget: No system tray detected!";
        icon = nullptr;
        this->show();
    }

    ui->statusbar->hide();
    ui->menubar->hide();

    layout()->setContentsMargins(0, 0, 0, 0);
    ui->friendList->setStyleSheet(Style::resolve(Style::getStylesheet(":ui/friendList/friendList.css")));

    profilePicture = new MaskablePixmapWidget(this, QSize(40, 40), ":/img/avatar_mask.png");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.png"));
    profilePicture->setClickable(true);
    ui->myProfile->insertWidget(0, profilePicture);
    ui->myProfile->insertSpacing(1, 7);

    ui->mainContent->setLayout(new QVBoxLayout());
    ui->mainHead->setLayout(new QVBoxLayout());
    ui->mainHead->layout()->setMargin(0);
    ui->mainHead->layout()->setSpacing(0);

    ui->tooliconsZone->setStyleSheet(Style::resolve("QPushButton{background-color:@themeDark;border:none;}QPushButton:hover{background-color:@themeMediumDark;border:none;}"));
    
    if (QStyleFactory::keys().contains(Settings::getInstance().getStyle())
            && Settings::getInstance().getStyle() != "None")
    {
        ui->mainHead->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
        ui->mainContent->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
    }
    
    ui->mainHead->setStyleSheet(Style::getStylesheet(":ui/settings/mainHead.css"));    
    ui->mainContent->setStyleSheet(Style::getStylesheet(":ui/settings/mainContent.css"));
    
    ui->statusHead->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));

    contactListWidget = new FriendListWidget();
    ui->friendList->setWidget(contactListWidget);
    ui->friendList->setLayoutDirection(Qt::RightToLeft);

    ui->nameLabel->setEditable(true);
    ui->statusLabel->setEditable(true);

    ui->statusPanel->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));

    QMenu *statusButtonMenu = new QMenu(ui->statusButton);
    statusButtonMenu->addAction(statusOnline);
    statusButtonMenu->addAction(statusAway);
    statusButtonMenu->addAction(statusBusy);
    ui->statusButton->setMenu(statusButtonMenu);

    // disable proportional scaling
    ui->mainSplitter->setStretchFactor(0,0);
    ui->mainSplitter->setStretchFactor(1,1);

    ui->statusButton->setProperty("status", "offline");
    Style::repolish(ui->statusButton);

    // Disable some widgets until we're connected to the DHT
    ui->statusButton->setEnabled(false);

    Style::setThemeColor(Settings::getInstance().getThemeColor());
    reloadTheme();
    
    filesForm = new FilesForm();
    addFriendForm = new AddFriendForm;
    settingsWidget = new SettingsWidget();

    Core* core = Nexus::getCore();
    connect(core, SIGNAL(fileDownloadFinished(const QString&)), filesForm, SLOT(onFileDownloadComplete(const QString&)));
    connect(core, SIGNAL(fileUploadFinished(const QString&)), filesForm, SLOT(onFileUploadComplete(const QString&)));
    connect(settingsWidget, &SettingsWidget::setShowSystemTray, this, &Widget::onSetShowSystemTray);
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(onAddClicked()));
    connect(ui->groupButton, SIGNAL(clicked()), this, SLOT(onGroupClicked()));
    connect(ui->transferButton, SIGNAL(clicked()), this, SLOT(onTransferClicked()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onSettingsClicked()));
    connect(ui->nameLabel, SIGNAL(textChanged(QString, QString)), this, SLOT(onUsernameChanged(QString, QString)));
    connect(ui->statusLabel, SIGNAL(textChanged(QString, QString)), this, SLOT(onStatusMessageChanged(QString, QString)));
    connect(ui->mainSplitter, &QSplitter::splitterMoved, this, &Widget::onSplitterMoved);
    connect(profilePicture, SIGNAL(clicked()), this, SLOT(onAvatarClicked()));
    connect(addFriendForm, SIGNAL(friendRequested(QString, QString)), this, SIGNAL(friendRequested(QString, QString)));
    connect(timer, &QTimer::timeout, this, &Widget::onUserAwayCheck);
    connect(timer, &QTimer::timeout, this, &Widget::onEventIconTick);
    connect(offlineMsgTimer, &QTimer::timeout, this, &Widget::processOfflineMsgs);

    addFriendForm->show(*ui);

#if (AUTOUPDATE_ENABLED)
    if (Settings::getInstance().getCheckUpdates())
        AutoUpdater::checkUpdatesAsyncInteractive();
#endif
}

void Widget::setTranslation()
{
    // Load translations
    QCoreApplication::removeTranslator(translator);
    QString locale;
    if ((locale = Settings::getInstance().getTranslation()).isEmpty())
        locale = QLocale::system().name().section('_', 0, 0);
    
    if (locale == "en")
        return;

    if (translator->load(locale, ":translations/"))
        qDebug() << "Loaded translation" << locale;
    else
        qDebug() << "Error loading translation" << locale;
    QCoreApplication::installTranslator(translator);
}

void Widget::updateTrayIcon()
{
    QString status;
    if (eventIcon)
        status = "event";
    else
    {
        status = ui->statusButton->property("status").toString();
        if (!status.length())
            status = "offline";
    }
    QString color = Settings::getInstance().getLightTrayIcon() ? "light" : "dark";
    QString pic = ":img/taskbar/" + color + "/taskbar_" + status + ".svg";
    if (icon)
        icon->setIcon(QIcon(pic));
}

Widget::~Widget()
{
    qDebug() << "Widget: Deleting Widget";
    AutoUpdater::abortUpdates();
    icon->hide();
    hideMainForms();
    delete settingsWidget;
    delete addFriendForm;
    delete filesForm;
    delete timer;
    delete offlineMsgTimer;

    FriendList::clear();
    GroupList::clear();
    delete trayMenu;
    delete ui;
    delete translator;
    instance = nullptr;
}

Widget* Widget::getInstance()
{
    assert(IS_ON_DESKTOP_GUI); // Widget must only be used on Desktop platforms

    if (!instance)
    {
        instance = new Widget();
        instance->init();
    }
    return instance;
}

void Widget::closeEvent(QCloseEvent *event)
{
    if (Settings::getInstance().getShowSystemTray() && Settings::getInstance().getCloseToTray() == true)
    {
        event->ignore();
        this->hide();
    }
    else
    {
        saveWindowGeometry();
        saveSplitterGeometry();
        qApp->exit(0);
        QWidget::closeEvent(event);
    }
}

void Widget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (isMinimized() && Settings::getInstance().getMinimizeToTray())
        {
            this->hide();
        }
    }
}

void Widget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    saveWindowGeometry();

    emit resized();
}

QString Widget::getUsername()
{
    return Nexus::getCore()->getUsername();
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

    Nexus::getCore()->setAvatar(TOX_AVATAR_FORMAT_PNG, bytes);
}

void Widget::onSelfAvatarLoaded(const QPixmap& pic)
{
    profilePicture->setPixmap(pic);
}

void Widget::onConnected()
{
    ui->statusButton->setEnabled(true);
    if (beforeDisconnect == Status::Offline)
        emit statusSet(Status::Online);
    else
        emit statusSet(beforeDisconnect);
}

void Widget::onDisconnected()
{
    QString stat = ui->statusButton->property("status").toString();
    if      (stat == "online")
        beforeDisconnect = Status::Online;
    else if (stat == "busy")
        beforeDisconnect = Status::Busy;
    else if (stat == "away")
        beforeDisconnect = Status::Away;
    else
        beforeDisconnect = Status::Offline;

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
        ui->statusButton->setIcon(QIcon(":img/status/dot_online.png"));
        break;
    case Status::Away:
        ui->statusButton->setProperty("status" ,"away");
        ui->statusButton->setIcon(QIcon(":img/status/dot_idle.png"));
        break;
    case Status::Busy:
        ui->statusButton->setProperty("status" ,"busy");
        ui->statusButton->setIcon(QIcon(":img/status/dot_busy.png"));
        break;
    case Status::Offline:
        ui->statusButton->setProperty("status" ,"offline");
        ui->statusButton->setIcon(QIcon(":img/status/dot_away.png"));
        break;
    }
    updateTrayIcon();
}

void Widget::setWindowTitle(const QString& title)
{
    QMainWindow::setWindowTitle("qTox - " + title);
}

void Widget::forceShow()
{
    hide();                     // Workaround to force minimized window to be restored
    show();
    activateWindow();
}

void Widget::onAddClicked()
{
    hideMainForms();
    addFriendForm->show(*ui);
    setWindowTitle(tr("Add friend"));
}

void Widget::onGroupClicked()
{
    Nexus::getCore()->createGroup();
}

void Widget::onTransferClicked()
{
    hideMainForms();
    filesForm->show(*ui);
    setWindowTitle(tr("File transfers"));
    activeChatroomWidget = nullptr;
}

void Widget::onIconClick(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
        {
            if (isHidden())
            {
                show();
                activateWindow();
            }
            else if (isMinimized())
            {
                forceShow();
            }
            else
            {
                hide();
            }

            break;
        }
        case QSystemTrayIcon::DoubleClick:
            forceShow();
            break;
        case QSystemTrayIcon::MiddleClick:
            hide();
            break;
        case QSystemTrayIcon::Unknown:
            if (isHidden())
                forceShow();
            break;
        default:
            break;
    }
}

void Widget::onSettingsClicked()
{
    hideMainForms();
    settingsWidget->show(*ui);
    setWindowTitle(tr("Settings"));
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
    setUsername(oldUsername);               // restore old username until Core tells us to set it
    Nexus::getCore()->setUsername(newUsername);
}

void Widget::setUsername(const QString& username)
{
    ui->nameLabel->setText(username);
    ui->nameLabel->setToolTip(username);    // for overlength names
    QString sanename = username;
    sanename.remove(QRegExp("[\\t\\n\\v\\f\\r\\x0000]"));
             nameMention = QRegExp("\\b" + QRegExp::escape(username) + "\\b", Qt::CaseInsensitive);
    sanitizedNameMention = QRegExp("\\b" + QRegExp::escape(sanename) + "\\b", Qt::CaseInsensitive);
}

void Widget::onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage)
{
    ui->statusLabel->setText(oldStatusMessage); // restore old status message until Core tells us to set it
    ui->statusLabel->setToolTip(oldStatusMessage); // for overlength messsages
    Nexus::getCore()->setStatusMessage(newStatusMessage);
}

void Widget::setStatusMessage(const QString &statusMessage)
{
    ui->statusLabel->setText(statusMessage);
    ui->statusLabel->setToolTip(statusMessage); // for overlength messsages
}

void Widget::reloadHistory()
{
    for (auto f : FriendList::getAllFriends())
        f->getChatForm()->loadHistory(QDateTime::currentDateTime().addDays(-7), true);
}

void Widget::addFriend(int friendId, const QString &userId)
{
    //qDebug() << "Widget: Adding friend with id" << userId;
    ToxID userToxId = ToxID::fromString(userId);
    Friend* newfriend = FriendList::addFriend(friendId, userToxId);
    QLayout* layout = contactListWidget->getFriendLayout(Status::Offline);
    layout->addWidget(newfriend->getFriendWidget());

    if (Settings::getInstance().getEnableLogging())
        newfriend->getChatForm()->loadHistory(QDateTime::currentDateTime().addDays(-7), true);

    Core* core = Nexus::getCore();
    connect(settingsWidget, &SettingsWidget::compactToggled, newfriend->getFriendWidget(), &GenericChatroomWidget::onCompactChanged);
    connect(newfriend->getFriendWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), this, SLOT(onChatroomWidgetClicked(GenericChatroomWidget*)));
    connect(newfriend->getFriendWidget(), SIGNAL(removeFriend(int)), this, SLOT(removeFriend(int)));
    connect(newfriend->getFriendWidget(), SIGNAL(copyFriendIdToClipboard(int)), this, SLOT(copyFriendIdToClipboard(int)));
    connect(newfriend->getFriendWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), newfriend->getChatForm(), SLOT(focusInput()));
    connect(newfriend->getChatForm(), SIGNAL(sendMessage(int,QString)), core, SLOT(sendMessage(int,QString)));
    connect(newfriend->getChatForm(), &GenericChatForm::sendAction, core, &Core::sendAction);
    connect(newfriend->getChatForm(), SIGNAL(sendFile(int32_t, QString, QString, long long)), core, SLOT(sendFile(int32_t, QString, QString, long long)));
    connect(newfriend->getChatForm(), SIGNAL(answerCall(int)), core, SLOT(answerCall(int)));
    connect(newfriend->getChatForm(), SIGNAL(hangupCall(int)), core, SLOT(hangupCall(int)));
    connect(newfriend->getChatForm(), SIGNAL(rejectCall(int)), core, SLOT(rejectCall(int)));
    connect(newfriend->getChatForm(), SIGNAL(startCall(int)), core, SLOT(startCall(int)));
    connect(newfriend->getChatForm(), SIGNAL(startVideoCall(int,bool)), core, SLOT(startCall(int,bool)));
    connect(newfriend->getChatForm(), SIGNAL(cancelCall(int,int)), core, SLOT(cancelCall(int,int)));
    connect(newfriend->getChatForm(), SIGNAL(micMuteToggle(int)), core, SLOT(micMuteToggle(int)));
    connect(newfriend->getChatForm(), SIGNAL(volMuteToggle(int)), core, SLOT(volMuteToggle(int)));
    connect(newfriend->getChatForm(), &ChatForm::aliasChanged, newfriend->getFriendWidget(), &FriendWidget::setAlias);
    connect(core, &Core::fileReceiveRequested, newfriend->getChatForm(), &ChatForm::onFileRecvRequest);
    connect(core, &Core::avInvite, newfriend->getChatForm(), &ChatForm::onAvInvite);
    connect(core, &Core::avStart, newfriend->getChatForm(), &ChatForm::onAvStart);
    connect(core, &Core::avCancel, newfriend->getChatForm(), &ChatForm::onAvCancel);
    connect(core, &Core::avEnd, newfriend->getChatForm(), &ChatForm::onAvEnd);
    connect(core, &Core::avRinging, newfriend->getChatForm(), &ChatForm::onAvRinging);
    connect(core, &Core::avStarting, newfriend->getChatForm(), &ChatForm::onAvStarting);
    connect(core, &Core::avEnding, newfriend->getChatForm(), &ChatForm::onAvEnding);
    connect(core, &Core::avRequestTimeout, newfriend->getChatForm(), &ChatForm::onAvRequestTimeout);
    connect(core, &Core::avPeerTimeout, newfriend->getChatForm(), &ChatForm::onAvPeerTimeout);
    connect(core, &Core::avMediaChange, newfriend->getChatForm(), &ChatForm::onAvMediaChange);
    connect(core, &Core::avCallFailed, newfriend->getChatForm(), &ChatForm::onAvCallFailed);
    connect(core, &Core::avRejected, newfriend->getChatForm(), &ChatForm::onAvRejected);
    connect(core, &Core::friendAvatarChanged, newfriend->getChatForm(), &ChatForm::onAvatarChange);
    connect(core, &Core::friendAvatarChanged, newfriend->getFriendWidget(), &FriendWidget::onAvatarChange);
    connect(core, &Core::friendAvatarRemoved, newfriend->getChatForm(), &ChatForm::onAvatarRemoved);
    connect(core, &Core::friendAvatarRemoved, newfriend->getFriendWidget(), &FriendWidget::onAvatarRemoved);

    // Try to get the avatar from the cache
    QPixmap avatar = Settings::getInstance().getSavedAvatar(userId);
    if (!avatar.isNull())
    {
        //qWarning() << "Widget: loadded avatar for id" << userId;
        newfriend->getChatForm()->onAvatarChange(friendId, avatar);
        newfriend->getFriendWidget()->onAvatarChange(friendId, avatar);
    }
}

void Widget::addFriendFailed(const QString&, const QString& errorInfo)
{
    QString info = QString(tr("Couldn't request friendship"));
    if (!errorInfo.isEmpty()) {
        info = info + (QString(": ") + errorInfo);
    }

    QMessageBox::critical(0,"Error",info);
}

void Widget::onFriendStatusChanged(int friendId, Status status)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    contactListWidget->moveWidget(f->getFriendWidget(), status, f->getEventFlag());

    bool isActualChange = f->getStatus() != status;

    f->setStatus(status);
    f->getFriendWidget()->updateStatusLight();
    
    //won't print the message if there were no messages before
    if (!f->getChatForm()->isEmpty()
            && Settings::getInstance().getStatusChangeNotificationEnabled())
    {
        QString fStatus = "";
        switch(f->getStatus()){
        case Status::Away:
            fStatus = tr("away", "contact status"); break;
        case Status::Busy:
            fStatus = tr("busy", "contact status"); break;
        case Status::Offline:
            fStatus = tr("offline", "contact status"); break;
        default:
            fStatus = tr("online", "contact status"); break;
        }
        if (isActualChange)
            f->getChatForm()->addSystemInfoMessage(tr("%1 is now %2", "e.g. \"Dubslow is now online\"").arg(f->getDisplayedName()).arg(fStatus),
                                          "white", QDateTime::currentDateTime());
    }

    if (isActualChange && status != Status::Offline)
    { // wait a little
        QTimer::singleShot(250, f->getChatForm()->getOfflineMsgEngine(), SLOT(deliverOfflineMsgs()));
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
    setWindowTitle(widget->getName());
    widget->resetEventFlags();
    widget->updateStatusLight();
}

void Widget::onFriendMessageReceived(int friendId, const QString& message, bool isAction)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    QDateTime timestamp = QDateTime::currentDateTime();
    f->getChatForm()->addMessage(f->getToxID(), message, isAction, timestamp, true);

    HistoryKeeper::getInstance()->addChatEntry(f->getToxID().publicKey, isAction ? "/me " + message : message,
                                               f->getToxID().publicKey, timestamp, true);

    f->setEventFlag(static_cast<GenericChatroomWidget*>(f->getFriendWidget()) != activeChatroomWidget);
    newMessageAlert(f->getFriendWidget());
    f->getFriendWidget()->updateStatusLight();
}

void Widget::onReceiptRecieved(int friendId, int receipt)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->getChatForm()->getOfflineMsgEngine()->dischargeReceipt(receipt);
}

void Widget::newMessageAlert(GenericChatroomWidget* chat)
{
    bool inactiveWindow = isMinimized() || !isActiveWindow();
    if (!inactiveWindow && activeChatroomWidget != nullptr && chat == activeChatroomWidget)
        return;

    QApplication::alert(this);

    if (inactiveWindow)
        eventFlag = true;

    if (Settings::getInstance().getShowWindow())
    {
        show();
        if (inactiveWindow && Settings::getInstance().getShowInFront())
            setWindowState(Qt::WindowActive);
    }

    static QFile sndFile(":audio/notification.pcm");
    static QByteArray sndData;

    if (sndData.isEmpty())
    {
        sndFile.open(QIODevice::ReadOnly);
        sndData = sndFile.readAll();
        sndFile.close();
    }

    Audio::playMono16Sound(sndData);
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

    Audio::playMono16Sound(sndData1);
}

void Widget::onFriendRequestReceived(const QString& userId, const QString& message)
{
    FriendRequestDialog dialog(this, userId, message);

    if (dialog.exec() == QDialog::Accepted)
        emit friendRequestAccepted(userId);
}

void Widget::removeFriend(Friend* f, bool fake)
{
    f->getFriendWidget()->setAsInactiveChatroom();
    if (static_cast<GenericChatroomWidget*>(f->getFriendWidget()) == activeChatroomWidget)
    {
        activeChatroomWidget = nullptr;
        onAddClicked();
    }
    FriendList::removeFriend(f->getFriendID(), fake);
    Nexus::getCore()->removeFriend(f->getFriendID(), fake);
    delete f;
    if (ui->mainHead->layout()->isEmpty())
        onAddClicked();

    contactListWidget->hide();
    contactListWidget->show();
}

void Widget::removeFriend(int friendId)
{
    removeFriend(FriendList::findFriend(friendId), false);
}

void Widget::clearContactsList()
{
    QList<Friend*> friends = FriendList::getAllFriends();
    for (Friend* f : friends)
        removeFriend(f, true);

    QList<Group*> groups = GroupList::getAllGroups();
    for (Group* g : groups)
        removeGroup(g, true);
}

void Widget::copyFriendIdToClipboard(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    if (f != nullptr)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(Nexus::getCore()->getFriendAddress(f->getFriendID()), QClipboard::Clipboard);
    }
}

void Widget::onGroupInviteReceived(int32_t friendId, uint8_t type, QByteArray invite)
{
    if (type == TOX_GROUPCHAT_TYPE_TEXT || type == TOX_GROUPCHAT_TYPE_AV)
    {
        int groupId = Nexus::getCore()->joinGroupchat(friendId, type, (uint8_t*)invite.data(), invite.length());
        if (groupId < 0)
        {
            qWarning() << "Widget::onGroupInviteReceived: Unable to accept  group invite";
            return;
        }
    }
    else
    {
        qWarning() << "Widget::onGroupInviteReceived: Unknown groupchat type:"<<type;
        return;
    }
}

void Widget::onGroupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    ToxID author = Core::getInstance()->getGroupPeerToxID(groupnumber, peernumber);
    bool targeted = !author.isMine() && (message.contains(nameMention) || message.contains(sanitizedNameMention));
    if (targeted && !isAction)
        g->getChatForm()->addAlertMessage(author, message, QDateTime::currentDateTime());
    else
        g->getChatForm()->addMessage(author, message, isAction, QDateTime::currentDateTime(), true);

    g->setEventFlag(static_cast<GenericChatroomWidget*>(g->getGroupWidget()) != activeChatroomWidget);

    if (targeted || Settings::getInstance().getGroupAlwaysNotify())
        newMessageAlert(g->getGroupWidget());

    if (targeted)
        g->setMentionedFlag(true); // useful for highlighting line or desktop notifications

    g->getGroupWidget()->updateStatusLight();
}

void Widget::onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t Change)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
    {
        qDebug() << "Widget::onGroupNamelistChanged: Group "<<groupnumber<<" not found, creating it";
        g = createGroup(groupnumber);
    }

    QString name = Nexus::getCore()->getGroupPeerName(groupnumber, peernumber);
    TOX_CHAT_CHANGE change = static_cast<TOX_CHAT_CHANGE>(Change);
    if (change == TOX_CHAT_CHANGE_PEER_ADD)
    {
        if (name.isEmpty())
            name = tr("<Unknown>", "Placeholder when we don't know someone's name in a group chat");
        // g->addPeer(peernumber,name);
        g->regeneratePeerList();
        //g->chatForm->addSystemInfoMessage(tr("%1 has joined the chat").arg(name), "green");
        // we can't display these messages until irungentoo fixes peernumbers
        // https://github.com/irungentoo/toxcore/issues/1128
    }
    else if (change == TOX_CHAT_CHANGE_PEER_DEL)
    {
        // g->removePeer(peernumber);
        g->regeneratePeerList();
        //g->chatForm->addSystemInfoMessage(tr("%1 has left the chat").arg(name), "silver");
    }
    else if (change == TOX_CHAT_CHANGE_PEER_NAME) // core overwrites old name before telling us it changed...
        g->updatePeer(peernumber,Nexus::getCore()->getGroupPeerName(groupnumber, peernumber));
}

void Widget::onGroupTitleChanged(int groupnumber, const QString& author, const QString& title)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    g->setName(title);
    if (!author.isEmpty())
        g->getChatForm()->addSystemInfoMessage(tr("%1 has set the title to %2").arg(author, title), "silver", QDateTime::currentDateTime());
}

void Widget::removeGroup(Group* g, bool fake)
{
    g->getGroupWidget()->setAsInactiveChatroom();
    if (static_cast<GenericChatroomWidget*>(g->getGroupWidget()) == activeChatroomWidget)
    {
        activeChatroomWidget = nullptr;
        onAddClicked();
    }
    GroupList::removeGroup(g->getGroupId(), fake);
    Nexus::getCore()->removeGroup(g->getGroupId(), fake);
    delete g;
    if (ui->mainHead->layout()->isEmpty())
        onAddClicked();

    contactListWidget->hide();
    contactListWidget->show();
}

void Widget::removeGroup(int groupId)
{
    removeGroup(GroupList::findGroup(groupId));
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
    Group* newgroup = GroupList::addGroup(groupId, groupName, true);
    QLayout* layout = contactListWidget->getGroupLayout();
    layout->addWidget(newgroup->getGroupWidget());
    newgroup->getGroupWidget()->updateStatusLight();

    Core* core = Nexus::getCore();
    connect(newgroup->getGroupWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), this, SLOT(onChatroomWidgetClicked(GenericChatroomWidget*)));
    connect(newgroup->getGroupWidget(), SIGNAL(removeGroup(int)), this, SLOT(removeGroup(int)));
    connect(newgroup->getGroupWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), newgroup->getChatForm(), SLOT(focusInput()));
    connect(newgroup->getChatForm(), SIGNAL(sendMessage(int,QString)), core, SLOT(sendGroupMessage(int,QString)));
    connect(newgroup->getChatForm(), SIGNAL(sendAction(int,QString)), core, SLOT(sendGroupAction(int,QString)));
    connect(newgroup->getChatForm(), &GroupChatForm::groupTitleChanged, core, &Core::changeGroupTitle);
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

    return (activeChatroomWidget == static_cast<GenericChatroomWidget*>(f->getFriendWidget()));
}

bool Widget::event(QEvent * e)
{
    switch(e->type())
    {
        case QEvent::WindowActivate:
            if (activeChatroomWidget != nullptr)
            {
                activeChatroomWidget->resetEventFlags();
                activeChatroomWidget->updateStatusLight();
            }
            if (eventFlag)
            {
                eventFlag = false;
                eventIcon = false;
                updateTrayIcon();
            }
        default:
            break;
    }

    return QWidget::event(e);
}

void Widget::onUserAwayCheck()
{
#ifdef QTOX_PLATFORM_EXT
    uint32_t autoAwayTime = Settings::getInstance().getAutoAwayTime() * 60 * 1000;

    if (ui->statusButton->property("status").toString() == "online")
    {
        if (autoAwayTime && Platform::getIdleTime() >= autoAwayTime)
        {
            qDebug() << "Widget: auto away activated at" << QTime::currentTime().toString();
            emit statusSet(Status::Away);
            autoAwayActive = true;
        }
    }
    else if (ui->statusButton->property("status").toString() == "away")
    {
        if (autoAwayActive && (!autoAwayTime || Platform::getIdleTime() < autoAwayTime))
        {
            qDebug() << "Widget: auto away deactivated at" << QTime::currentTime().toString();
            emit statusSet(Status::Online);
            autoAwayActive = false;
        }
    }
    else if (autoAwayActive)
        autoAwayActive = false;
#endif
}

void Widget::onEventIconTick()
{
    if (eventFlag)
    {
        eventIcon ^= true;
        updateTrayIcon();
    }
}

void Widget::setStatusOnline()
{
    Nexus::getCore()->setStatus(Status::Online);
}

void Widget::setStatusAway()
{
    Nexus::getCore()->setStatus(Status::Away);
}

void Widget::setStatusBusy()
{
    Nexus::getCore()->setStatus(Status::Busy);
}

void Widget::onMessageSendResult(int friendId, const QString& message, int messageId)
{
    Q_UNUSED(message)
    Q_UNUSED(messageId)
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;
}

void Widget::onGroupSendResult(int groupId, const QString& message, int result)
{
    Q_UNUSED(message)
    Group* g = GroupList::findGroup(groupId);
    if (!g)
        return;

    if (result == -1)
        g->getChatForm()->addSystemInfoMessage(tr("Message failed to send"), "red", QDateTime::currentDateTime());
}

void Widget::onFriendTypingChanged(int friendId, bool isTyping)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;
    f->getChatForm()->setFriendTyping(isTyping);
}

void Widget::onSetShowSystemTray(bool newValue){
    icon->setVisible(newValue);
}

void Widget::saveWindowGeometry()
{
    Settings::getInstance().setWindowGeometry(saveGeometry());
    Settings::getInstance().setWindowState(saveState());
}

void Widget::saveSplitterGeometry()
{
    Settings::getInstance().setSplitterState(ui->mainSplitter->saveState());
}

void Widget::onSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);
    saveSplitterGeometry();
}

void Widget::processOfflineMsgs()
{
    if (OfflineMsgEngine::globalMutex.tryLock())
    {
        QList<Friend*> frnds = FriendList::getAllFriends();
        for (Friend *f : frnds)
        {
            f->getChatForm()->getOfflineMsgEngine()->deliverOfflineMsgs();
        }

        OfflineMsgEngine::globalMutex.unlock();
    }
}

void Widget::clearAllReceipts()
{
    QList<Friend*> frnds = FriendList::getAllFriends();
    for (Friend *f : frnds)
    {
        f->getChatForm()->getOfflineMsgEngine()->removeAllReciepts();
    }
}

void Widget::reloadTheme()
{
    QString statusPanelStyle = Style::getStylesheet(":/ui/window/statusPanel.css");
    ui->tooliconsZone->setStyleSheet(Style::resolve("QPushButton{background-color:@themeDark;border:none;}QPushButton:hover{background-color:@themeMediumDark;border:none;}"));
    ui->statusPanel->setStyleSheet(statusPanelStyle);
    ui->statusHead->setStyleSheet(statusPanelStyle);
    ui->friendList->setStyleSheet(Style::getStylesheet(":ui/friendList/friendList.css"));
    ui->statusButton->setStyleSheet(Style::getStylesheet(":ui/statusButton/statusButton.css"));

    for (Friend* f : FriendList::getAllFriends())
        f->getFriendWidget()->reloadTheme();

    for (Group* g : GroupList::getAllGroups())
        g->getGroupWidget()->reloadTheme();
}
