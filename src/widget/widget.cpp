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

#include "widget.h"
#include "ui_mainwindow.h"
#include "src/core/core.h"
#include "src/persistence/settings.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "tool/friendrequestdialog.h"
#include "friendwidget.h"
#include "src/grouplist.h"
#include "src/group.h"
#include "groupwidget.h"
#include "form/groupchatform.h"
#include "src/widget/style.h"
#include "friendlistwidget.h"
#include "form/chatform.h"
#include "maskablepixmapwidget.h"
#include "src/persistence/historykeeper.h"
#include "src/net/autoupdate.h"
#include "src/audio/audio.h"
#include "src/platform/timer.h"
#include "systemtrayicon.h"
#include "src/nexus.h"
#include "src/widget/gui.h"
#include "src/persistence/offlinemsgengine.h"
#include "src/widget/translator.h"
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
#include <QDialogButtonBox>
#include <QShortcut>
#include <QTimer>
#include <QStyleFactory>
#include <QString>
#include <QByteArray>
#include <QImageReader>
#include <QList>
#include <QDesktopServices>
#include <QProcess>
#include <tox/tox.h>

#ifdef Q_OS_ANDROID
#define IS_ON_DESKTOP_GUI 0
#else
#define IS_ON_DESKTOP_GUI 1
#endif

bool toxActivateEventHandler(const QByteArray&)
{
    if (!Widget::getInstance()->isActiveWindow())
        Widget::getInstance()->forceShow();

    return true;
}

Widget *Widget::instance{nullptr};

Widget::Widget(QWidget *parent)
    : QMainWindow(parent),
      icon{nullptr},
      trayMenu{nullptr},
      ui(new Ui::MainWindow),
      activeChatroomWidget{nullptr},
      eventFlag(false),
      eventIcon(false)
{
    installEventFilter(this);
    Translator::translate();
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

    statusOnline = new QAction(this);
    statusOnline->setIcon(getStatusIcon(Status::Online, 10, 10));
    connect(statusOnline, SIGNAL(triggered()), this, SLOT(setStatusOnline()));
    statusAway = new QAction(this);
    statusAway->setIcon(getStatusIcon(Status::Away, 10, 10));
    connect(statusAway, SIGNAL(triggered()), this, SLOT(setStatusAway()));
    statusBusy = new QAction(this);
    statusBusy->setIcon(getStatusIcon(Status::Busy, 10, 10));
    connect(statusBusy, SIGNAL(triggered()), this, SLOT(setStatusBusy()));

    ui->statusbar->hide();
    ui->menubar->hide();

    layout()->setContentsMargins(0, 0, 0, 0);
    ui->friendList->setStyleSheet(Style::resolve(Style::getStylesheet(":ui/friendList/friendList.css")));

    profilePicture = new MaskablePixmapWidget(this, QSize(40, 40), ":/img/avatar_mask.svg");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.svg"));
    profilePicture->setClickable(true);
    ui->myProfile->insertWidget(0, profilePicture);
    ui->myProfile->insertSpacing(1, 7);

    ui->mainContent->setLayout(new QVBoxLayout());
    ui->mainHead->setLayout(new QVBoxLayout());
    ui->mainHead->layout()->setMargin(0);
    ui->mainHead->layout()->setSpacing(0);

    if (QStyleFactory::keys().contains(Settings::getInstance().getStyle())
            && Settings::getInstance().getStyle() != "None")
    {
        ui->mainHead->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
        ui->mainContent->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
    }

#ifndef Q_OS_MAC
    ui->mainHead->setStyleSheet(Style::getStylesheet(":ui/settings/mainHead.css"));
    ui->mainContent->setStyleSheet(Style::getStylesheet(":ui/settings/mainContent.css"));
    ui->statusHead->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));
    ui->statusPanel->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));
#endif

    contactListWidget = new FriendListWidget(0, Settings::getInstance().getGroupchatPosition());
    ui->friendList->setWidget(contactListWidget);
    ui->friendList->setLayoutDirection(Qt::RightToLeft);

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

    onStatusSet(Status::Offline);

    // Disable some widgets until we're connected to the DHT
    ui->statusButton->setEnabled(false);

    Style::setThemeColor(Settings::getInstance().getThemeColor());
    reloadTheme();
    updateIcons();

    filesForm = new FilesForm();
    addFriendForm = new AddFriendForm;
    profileForm = new ProfileForm();
    settingsWidget = new SettingsWidget();

    Core* core = Nexus::getCore();
    connect(core, &Core::fileDownloadFinished, filesForm, &FilesForm::onFileDownloadComplete);
    connect(core, &Core::fileUploadFinished, filesForm, &FilesForm::onFileUploadComplete);
    connect(settingsWidget, &SettingsWidget::setShowSystemTray, this, &Widget::onSetShowSystemTray);
    connect(core, &Core::selfAvatarChanged, profileForm, &ProfileForm::onSelfAvatarLoaded);
    connect(ui->addButton, &QPushButton::clicked, this, &Widget::onAddClicked);
    connect(ui->groupButton, &QPushButton::clicked, this, &Widget::onGroupClicked);
    connect(ui->transferButton, &QPushButton::clicked, this, &Widget::onTransferClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &Widget::onSettingsClicked);
    connect(profilePicture, &MaskablePixmapWidget::clicked, this, &Widget::showProfile);
    connect(ui->nameLabel, &CroppingLabel::clicked, this, &Widget::showProfile);
    connect(ui->statusLabel, &CroppingLabel::textChanged, this, &Widget::onStatusMessageChanged);
    connect(ui->mainSplitter, &QSplitter::splitterMoved, this, &Widget::onSplitterMoved);
    connect(addFriendForm, &AddFriendForm::friendRequested, this, &Widget::friendRequested);
    connect(timer, &QTimer::timeout, this, &Widget::onUserAwayCheck);
    connect(timer, &QTimer::timeout, this, &Widget::onEventIconTick);
    connect(timer, &QTimer::timeout, this, &Widget::onTryCreateTrayIcon);
    connect(offlineMsgTimer, &QTimer::timeout, this, &Widget::processOfflineMsgs);
    connect(ui->searchContactText, &QLineEdit::textChanged, this, &Widget::searchContacts);
    connect(ui->searchContactFilterCBox, &QComboBox::currentTextChanged, this, &Widget::searchContacts);

    // keyboard shortcuts
    new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(close()));
    new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_Tab, this, SLOT(nextContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageUp, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageDown, this, SLOT(nextContact()));

    addFriendForm->show(*ui);
    setWindowTitle(tr("Add friend"));
    ui->addButton->setCheckable(true);
    ui->transferButton->setCheckable(true);
    ui->settingsButton->setCheckable(true);
    setActiveToolMenuButton(Widget::AddButton);

    connect(settingsWidget, &SettingsWidget::groupchatPositionToggled, contactListWidget, &FriendListWidget::onGroupchatPositionChanged);
#if (AUTOUPDATE_ENABLED)
    if (Settings::getInstance().getCheckUpdates())
        AutoUpdater::checkUpdatesAsyncInteractive();
#endif

    retranslateUi();
    Translator::registerHandler(std::bind(&Widget::retranslateUi, this), this);

    if (!Settings::getInstance().getShowSystemTray())
        show();
}

bool Widget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange && obj != NULL)
    {
           QWindowStateChangeEvent * ce = static_cast<QWindowStateChangeEvent*>(event);
           if (windowState() & Qt::WindowMinimized)
           {
                if (ce->oldState() & Qt::WindowMaximized)
                    wasMaximized = true;
                else
                    wasMaximized = false;
           }
    }
    return false;
}

void Widget::updateIcons()
{
    if (!icon)
        return;

    QString status;
    if (eventIcon)
    {
        status = "event";
    }
    else
    {
        status = ui->statusButton->property("status").toString();
        if (!status.length())
            status = "offline";
    }

    QIcon ico = QIcon::fromTheme("qtox-" + status);
    if (ico.isNull())
    {
        QString color = Settings::getInstance().getLightTrayIcon() ? "light" : "dark";
        ico = QIcon(":img/taskbar/" + color + "/taskbar_" + status + ".svg");
    }

    setWindowIcon(ico);
    if (icon)
        icon->setIcon(ico);
}

Widget::~Widget()
{
    qDebug() << "Deleting Widget";
    Translator::unregister(this);
    AutoUpdater::abortUpdates();
    if (icon)
        icon->hide();

    hideMainForms();
    delete profileForm;
    delete settingsWidget;
    delete addFriendForm;
    delete filesForm;
    delete timer;
    delete offlineMsgTimer;

    FriendList::clear();
    GroupList::clear();
    delete trayMenu;
    delete ui;
    instance = nullptr;
}

Widget* Widget::getInstance()
{
    assert(IS_ON_DESKTOP_GUI); // Widget must only be used on Desktop platforms

    if (!instance)
        instance = new Widget();

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
        if (isMinimized() &&
                Settings::getInstance().getShowSystemTray() &&
                Settings::getInstance().getMinimizeToTray())
            this->hide();
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
    beforeDisconnect = getStatusFromString(ui->statusButton->property("status").toString());
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
    Settings::getInstance().setProxyType(0);
    QMessageBox critical(this);
    critical.setText(tr("toxcore failed to start with your proxy settings. qTox cannot run; please modify your "
               "settings and restart.", "popup text"));
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    onSettingsClicked();
}

void Widget::onStatusSet(Status status)
{
    ui->statusButton->setProperty("status", getStatusTitle(status));
    ui->statusButton->setIcon(getStatusIcon(status, 10, 10));
    updateIcons();
}

void Widget::setWindowTitle(const QString& title)
{
    QString tmp = title;
    /// <[^>]*> Regexp to remove HTML tags, in case someone used them in title
    QMainWindow::setWindowTitle("qTox - " + tmp.remove(QRegExp("<[^>]*>")));
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
    setActiveToolMenuButton(Widget::AddButton);
    activeChatroomWidget = nullptr;
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
    setActiveToolMenuButton(Widget::TransferButton);
    activeChatroomWidget = nullptr;
}

void Widget::confirmExecutableOpen(const QFileInfo file)
{
    static const QStringList dangerousExtensions = { "app", "bat", "com", "cpl", "dmg", "exe", "hta", "jar", "js", "jse", "lnk", "msc", "msh", "msh1", "msh1xml", "msh2", "msh2xml", "mshxml", "msi", "msp", "pif", "ps1", "ps1xml", "ps2", "ps2xml", "psc1", "psc2", "py", "reg", "scf", "sh", "src", "vb", "vbe", "vbs", "ws", "wsc", "wsf", "wsh" };

    if (dangerousExtensions.contains(file.suffix()))
    {
        if (!GUI::askQuestion(tr("Executable file", "popup title"), tr("You have asked qTox to open an executable file. Executable files can potentially damage your computer. Are you sure want to open this file?", "popup text"), false, true))
            return;

        // The user wants to run this file, so make it executable and run it
        QFile(file.filePath()).setPermissions(file.permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
        QProcess::startDetached(file.filePath());
    }
    else
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(file.filePath()));
    }
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
                if (wasMaximized)
                    showMaximized();
                else
                    showNormal();
            }
            else if (isMinimized())
            {
                forceShow();
                activateWindow();
                if (wasMaximized)
                    showMaximized();
                else
                    showNormal();
            }
            else
            {
                wasMaximized = isMaximized();
                if (Settings::getInstance().getMinimizeToTray())
                    hide();
                else
                    showMinimized();
            }

            break;
        }
        case QSystemTrayIcon::MiddleClick:
            wasMaximized = isMaximized();
            if (Settings::getInstance().getMinimizeToTray())
                hide();
            else
                showMinimized();
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
    setActiveToolMenuButton(Widget::SettingButton);
    activeChatroomWidget = nullptr;
}

void Widget::showProfile() // onAvatarClicked, onUsernameClicked
{
    hideMainForms();
    profileForm->show(*ui);
    setWindowTitle(tr("Profile"));
    activeChatroomWidget = nullptr;
}

void Widget::hideMainForms()
{
    setActiveToolMenuButton(Widget::None);
    QLayoutItem* item;
    while ((item = ui->mainHead->layout()->takeAt(0)) != 0)
        item->widget()->hide();

    while ((item = ui->mainContent->layout()->takeAt(0)) != 0)
        item->widget()->hide();

    if (activeChatroomWidget != nullptr)
        activeChatroomWidget->setAsInactiveChatroom();
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
    ToxId userToxId = ToxId(userId);
    Friend* newfriend = FriendList::addFriend(friendId, userToxId);
    contactListWidget->moveWidget(newfriend->getFriendWidget(),Status::Offline);

    Core* core = Nexus::getCore();
    connect(newfriend, &Friend::displayedNameChanged, contactListWidget, &FriendListWidget::moveWidget);
    connect(settingsWidget, &SettingsWidget::compactToggled, newfriend->getFriendWidget(), &GenericChatroomWidget::setCompact);
    connect(newfriend->getFriendWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), this, SLOT(onChatroomWidgetClicked(GenericChatroomWidget*)));
    connect(newfriend->getFriendWidget(), SIGNAL(removeFriend(int)), this, SLOT(removeFriend(int)));
    connect(newfriend->getFriendWidget(), SIGNAL(copyFriendIdToClipboard(int)), this, SLOT(copyFriendIdToClipboard(int)));
    connect(newfriend->getFriendWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), newfriend->getChatForm(), SLOT(focusInput()));
    connect(newfriend->getChatForm(), &GenericChatForm::sendMessage, core, &Core::sendMessage);
    connect(newfriend->getChatForm(), &GenericChatForm::sendAction, core, &Core::sendAction);
    connect(newfriend->getChatForm(), &ChatForm::sendFile, core, &Core::sendFile);
    connect(newfriend->getChatForm(), &ChatForm::answerCall, core, &Core::answerCall);
    connect(newfriend->getChatForm(), &ChatForm::hangupCall, core, &Core::hangupCall);
    connect(newfriend->getChatForm(), &ChatForm::rejectCall, core, &Core::rejectCall);
    connect(newfriend->getChatForm(), &ChatForm::startCall, core, &Core::startCall);
    connect(newfriend->getChatForm(), &ChatForm::cancelCall, core, &Core::cancelCall);
    connect(newfriend->getChatForm(), &ChatForm::micMuteToggle, core, &Core::micMuteToggle);
    connect(newfriend->getChatForm(), &ChatForm::volMuteToggle, core, &Core::volMuteToggle);
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
        newfriend->getChatForm()->onAvatarChange(friendId, avatar);
        newfriend->getFriendWidget()->onAvatarChange(friendId, avatar);
    }

    searchContacts();
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

    bool isActualChange = f->getStatus() != status;

    if (isActualChange)
    {
        if (f->getStatus() == Status::Offline)
            contactListWidget->moveWidget(f->getFriendWidget(), Status::Online);
        else if (status == Status::Offline)
            contactListWidget->moveWidget(f->getFriendWidget(), Status::Offline);
    }

    f->setStatus(status);
    f->getFriendWidget()->updateStatusLight();
    if(f->getFriendWidget()->isActive())
    {
        QString windowTitle = f->getFriendWidget()->getName();
        if (!f->getFriendWidget()->getStatusString().isNull())
            windowTitle += " (" + f->getFriendWidget()->getStatusString() + ")";
        setWindowTitle(windowTitle);
    }

    //won't print the message if there were no messages before
    if (!f->getChatForm()->isEmpty()
            && Settings::getInstance().getStatusChangeNotificationEnabled())
    {
        QString fStatus = "";
        switch (f->getStatus())
        {
        case Status::Away:
            fStatus = tr("away", "contact status"); break;
        case Status::Busy:
            fStatus = tr("busy", "contact status"); break;
        case Status::Offline:
            fStatus = tr("offline", "contact status");
            f->getChatForm()->setFriendTyping(false); // Hide the "is typing" message when a friend goes offline
            break;
        default:
            fStatus = tr("online", "contact status"); break;
        }
        if (isActualChange)
            f->getChatForm()->addSystemInfoMessage(tr("%1 is now %2", "e.g. \"Dubslow is now online\"").arg(f->getDisplayedName()).arg(fStatus),
                                                   ChatMessage::INFO, QDateTime::currentDateTime());
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
    searchContacts();
}

void Widget::onChatroomWidgetClicked(GenericChatroomWidget *widget)
{
    hideMainForms();
    widget->setChatForm(*ui);
    if (activeChatroomWidget != nullptr)
        activeChatroomWidget->setAsInactiveChatroom();

    activeChatroomWidget = widget;
    widget->setAsActiveChatroom();
    setWindowTitle(widget->getName());
    widget->resetEventFlags();
    widget->updateStatusLight();
    QString windowTitle = widget->getName();
    if (!widget->getStatusString().isNull())
        windowTitle += " (" + widget->getStatusString() + ")";
    setWindowTitle(windowTitle);
}

void Widget::onFriendMessageReceived(int friendId, const QString& message, bool isAction)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    QDateTime timestamp = QDateTime::currentDateTime();
    f->getChatForm()->addMessage(f->getToxId(), message, isAction, timestamp, true);

    HistoryKeeper::getInstance()->addChatEntry(f->getToxId().publicKey, isAction ? "/me " + f->getDisplayedName() + " " + message : message,
                                               f->getToxId().publicKey, timestamp, true);

    f->setEventFlag(f->getFriendWidget() != activeChatroomWidget);
    newMessageAlert(f->getFriendWidget());
    f->getFriendWidget()->updateStatusLight();
    if (f->getFriendWidget()->isActive())
    {
        QString windowTitle = f->getFriendWidget()->getName();
        if (!f->getFriendWidget()->getStatusString().isNull())
            windowTitle += " (" + f->getFriendWidget()->getStatusString() + ")";
        setWindowTitle(windowTitle);
    }
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

    if (ui->statusButton->property("status").toString() == "busy")
        return;

    QApplication::alert(this);

    if (inactiveWindow)
        eventFlag = true;

    if (Settings::getInstance().getShowWindow())
    {
        if (activeChatroomWidget != chat)
            onChatroomWidgetClicked(chat);
        show();
        if (inactiveWindow && Settings::getInstance().getShowInFront())
            setWindowState(Qt::WindowActive);
    }

    if (Settings::getInstance().getNotifySound())
    {
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
}

void Widget::playRingtone()
{
    if (ui->statusButton->property("status").toString() == "busy")
        return;

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
    if (!fake)
    {
        QMessageBox::StandardButton removeFriendMB;
        removeFriendMB = QMessageBox::question(0,
                                    tr("Remove history"),
                                    tr("Do you want to remove history as well?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (removeFriendMB == QMessageBox::Cancel)
               return;
        else if (removeFriendMB == QMessageBox::Yes)
            HistoryKeeper::getInstance()->removeFriendHistory(f->getToxId().publicKey);
    }

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

    contactListWidget->reDraw();
}

void Widget::removeFriend(int friendId)
{
    removeFriend(FriendList::findFriend(friendId), false);
}

void Widget::clearContactsList()
{
    assert(QThread::currentThread() == qApp->thread());

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
        if (GUI::askQuestion(tr("Group invite", "popup title"), tr("%1 has invited you to a groupchat. Would you like to join?", "popup text").arg(Nexus::getCore()->getFriendUsername(friendId)), true, false))
        {
            int groupId = Nexus::getCore()->joinGroupchat(friendId, type, (uint8_t*)invite.data(), invite.length());
            if (groupId < 0)
            {
                qWarning() << "onGroupInviteReceived: Unable to accept  group invite";
                return;
            }
        }
    }
    else
    {
        qWarning() << "onGroupInviteReceived: Unknown groupchat type:"<<type;
        return;
    }
}

void Widget::onGroupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    ToxId author = Core::getInstance()->getGroupPeerToxId(groupnumber, peernumber);
    bool targeted = !author.isActiveProfile() && (message.contains(nameMention) || message.contains(sanitizedNameMention));
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
    if (g->getGroupWidget()->isActive())
    {
        QString windowTitle = g->getGroupWidget()->getName();
        if (!g->getGroupWidget()->getStatusString().isNull())
            windowTitle += " (" + g->getGroupWidget()->getStatusString() + ")";
        setWindowTitle(windowTitle);
    }
}

void Widget::onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t Change)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
    {
        qDebug() << "onGroupNamelistChanged: Group "<<groupnumber<<" not found, creating it";
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
        // g->getChatForm()->addSystemInfoMessage(tr("%1 has joined the chat").arg(name), "white", QDateTime::currentDateTime());
        // we can't display these messages until irungentoo fixes peernumbers
        // https://github.com/irungentoo/toxcore/issues/1128
    }
    else if (change == TOX_CHAT_CHANGE_PEER_DEL)
    {
        // g->removePeer(peernumber);
        g->regeneratePeerList();
        // g->getChatForm()->addSystemInfoMessage(tr("%1 has left the chat").arg(name), "white", QDateTime::currentDateTime());
    }
    else if (change == TOX_CHAT_CHANGE_PEER_NAME) // core overwrites old name before telling us it changed...
    {
        g->updatePeer(peernumber,Nexus::getCore()->getGroupPeerName(groupnumber, peernumber));
    }
}

void Widget::onGroupTitleChanged(int groupnumber, const QString& author, const QString& title)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    g->setName(title);
    if (!author.isEmpty())
        g->getChatForm()->addSystemInfoMessage(tr("%1 has set the title to %2").arg(author, title), ChatMessage::INFO, QDateTime::currentDateTime());
    searchContacts();
}

void Widget::onGroupPeerAudioPlaying(int groupnumber, int peernumber)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    g->getChatForm()->peerAudioPlaying(peernumber);
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

    contactListWidget->reDraw();
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
        qWarning() << "createGroup: Group already exists";
        return g;
    }

    Core* core = Nexus::getCore();

    QString groupName = QString("Groupchat #%1").arg(groupId);
    Group* newgroup = GroupList::addGroup(groupId, groupName, core->isGroupAvEnabled(groupId));
    QLayout* layout = contactListWidget->getGroupLayout();
    layout->addWidget(newgroup->getGroupWidget());
    newgroup->getGroupWidget()->updateStatusLight();

    connect(settingsWidget, &SettingsWidget::compactToggled, newgroup->getGroupWidget(), &GenericChatroomWidget::setCompact);
    connect(newgroup->getGroupWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), this, SLOT(onChatroomWidgetClicked(GenericChatroomWidget*)));
    connect(newgroup->getGroupWidget(), SIGNAL(removeGroup(int)), this, SLOT(removeGroup(int)));
    connect(newgroup->getGroupWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), newgroup->getChatForm(), SLOT(focusInput()));
    connect(newgroup->getChatForm(), &GroupChatForm::sendMessage, core, &Core::sendGroupMessage);
    connect(newgroup->getChatForm(), &GroupChatForm::sendAction, core, &Core::sendGroupAction);
    connect(newgroup->getChatForm(), &GroupChatForm::groupTitleChanged, core, &Core::changeGroupTitle);
    searchContacts();
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
    switch (e->type())
    {
        case QEvent::WindowActivate:
            if (activeChatroomWidget != nullptr)
            {
                activeChatroomWidget->resetEventFlags();
                activeChatroomWidget->updateStatusLight();
                QString windowTitle = activeChatroomWidget->getName();
                if (!activeChatroomWidget->getStatusString().isNull())
                    windowTitle += " (" + activeChatroomWidget->getStatusString() + ")";
                setWindowTitle(windowTitle);
            }
            if (eventFlag)
            {
                eventFlag = false;
                eventIcon = false;
                updateIcons();
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
            qDebug() << "auto away activated at" << QTime::currentTime().toString();
            emit statusSet(Status::Away);
            autoAwayActive = true;
        }
    }
    else if (ui->statusButton->property("status").toString() == "away")
    {
        if (autoAwayActive && (!autoAwayTime || Platform::getIdleTime() < autoAwayTime))
        {
            qDebug() << "auto away deactivated at" << QTime::currentTime().toString();
            emit statusSet(Status::Online);
            autoAwayActive = false;
        }
    }
    else if (autoAwayActive)
    {
        autoAwayActive = false;
    }
#endif
}

void Widget::onEventIconTick()
{
    if (eventFlag)
    {
        eventIcon ^= true;
        updateIcons();
    }
}

void Widget::onTryCreateTrayIcon()
{
    static int32_t tries = 15;
    if (!icon && tries--)
    {
        if (QSystemTrayIcon::isSystemTrayAvailable())
        {
            icon = new SystemTrayIcon;
            updateIcons();
            trayMenu = new QMenu;

            actionQuit = new QAction(tr("&Quit"), this);
            connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

            trayMenu->addAction(statusOnline);
            trayMenu->addAction(statusAway);
            trayMenu->addAction(statusBusy);
            trayMenu->addSeparator();
            trayMenu->addAction(actionQuit);
            icon->setContextMenu(trayMenu);

            connect(icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                    this, SLOT(onIconClick(QSystemTrayIcon::ActivationReason)));

            if (Settings::getInstance().getShowSystemTray())
            {
                icon->show();
                setHidden(Settings::getInstance().getAutostartInTray());
            }
            else
            {
                show();
            }
        }
        else if (!isVisible())
        {
            show();
        }
    }
    else
    {
        disconnect(timer, &QTimer::timeout, this, &Widget::onTryCreateTrayIcon);
        if (!icon)
        {
            qWarning() << "No system tray detected!";
            show();
        }
    }
}

void Widget::setStatusOnline()
{
    if (!ui->statusButton->isEnabled())
        return;

    Nexus::getCore()->setStatus(Status::Online);
}

void Widget::setStatusAway()
{
    if (!ui->statusButton->isEnabled())
        return;

    Nexus::getCore()->setStatus(Status::Away);
}

void Widget::setStatusBusy()
{
    if (!ui->statusButton->isEnabled())
        return;

    Nexus::getCore()->setStatus(Status::Busy);
}

void Widget::onMessageSendResult(uint32_t friendId, const QString& message, int messageId)
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
        g->getChatForm()->addSystemInfoMessage(tr("Message failed to send"), ChatMessage::INFO, QDateTime::currentDateTime());
}

void Widget::onFriendTypingChanged(int friendId, bool isTyping)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->getChatForm()->setFriendTyping(isTyping);
}

void Widget::onSetShowSystemTray(bool newValue)
{
    if (icon)
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

void Widget::cycleContacts(int offset)
{
    if (!activeChatroomWidget)
        return;

    FriendListWidget* friendList = static_cast<FriendListWidget*>(ui->friendList->widget());
    QList<GenericChatroomWidget*> friends = friendList->getAllFriends();

    int activeIndex = friends.indexOf(activeChatroomWidget);
    int bounded = (activeIndex + offset) % friends.length();

    if(bounded < 0)
        bounded += friends.length();

    emit friends[bounded]->chatroomWidgetClicked(friends[bounded]);
}

void Widget::processOfflineMsgs()
{
    if (OfflineMsgEngine::globalMutex.tryLock())
    {
        QList<Friend*> frnds = FriendList::getAllFriends();
        for (Friend *f : frnds)
            f->getChatForm()->getOfflineMsgEngine()->deliverOfflineMsgs();

        OfflineMsgEngine::globalMutex.unlock();
    }
}

void Widget::clearAllReceipts()
{
    QList<Friend*> frnds = FriendList::getAllFriends();
    for (Friend *f : frnds)
        f->getChatForm()->getOfflineMsgEngine()->removeAllReciepts();
}

void Widget::reloadTheme()
{
    QString statusPanelStyle = Style::getStylesheet(":/ui/window/statusPanel.css");
    ui->tooliconsZone->setStyleSheet(Style::resolve("QPushButton{background-color:@themeDark;border:none;}QPushButton:hover{background-color:@themeMediumDark;border:none;}QPushButton:checked{background-color:@themeMedium;border:none;}QPushButton:pressed{background-color:@themeMediumLight;border:none;}"));
    ui->statusPanel->setStyleSheet(statusPanelStyle);
    ui->statusHead->setStyleSheet(statusPanelStyle);
    ui->friendList->setStyleSheet(Style::getStylesheet(":ui/friendList/friendList.css"));
    ui->statusButton->setStyleSheet(Style::getStylesheet(":ui/statusButton/statusButton.css"));

    for (Friend* f : FriendList::getAllFriends())
        f->getFriendWidget()->reloadTheme();

    for (Group* g : GroupList::getAllGroups())
        g->getGroupWidget()->reloadTheme();
}

void Widget::nextContact()
{
    cycleContacts(1);
}

void Widget::previousContact()
{
    cycleContacts(-1);
}

QString Widget::getStatusIconPath(Status status)
{
    switch (status)
    {
    case Status::Online:
        return ":img/status/dot_online.svg";
    case Status::Away:
        return ":img/status/dot_away.svg";
    case Status::Busy:
        return ":img/status/dot_busy.svg";
    case Status::Offline:
    default:
        return ":img/status/dot_offline.svg";
    }
}

inline QIcon Widget::getStatusIcon(Status status, uint32_t w/*=0*/, uint32_t h/*=0*/)
{
    if (w > 0 && h > 0)
        return getStatusIconPixmap(status, w, h);
    else
        return QIcon(getStatusIconPath(status));
}

QPixmap Widget::getStatusIconPixmap(Status status, uint32_t w, uint32_t h)
{
    QPixmap pix(w, h);
    pix.load(getStatusIconPath(status));
    return pix;
}

QString Widget::getStatusTitle(Status status)
{
    switch (status)
    {
    case Status::Online:
        return "online";
    case Status::Away:
        return "away";
    case Status::Busy:
        return "busy";
    case Status::Offline:
    default:
        return "offline";
    }
}

Status Widget::getStatusFromString(QString status)
{
    if (status == "online")
        return Status::Online;
    else if (status == "busy")
        return Status::Busy;
    else if (status == "away")
        return Status::Away;
    else
        return Status::Offline;
}

void Widget::searchContacts()
{
    QString searchString = ui->searchContactText->text();
    int filter = ui->searchContactFilterCBox->currentIndex();

    switch(filter)
    {
        case FilterCriteria::All:
            hideFriends(searchString, Status::Online);
            hideFriends(searchString, Status::Offline);

            hideGroups(searchString);
            break;
        case FilterCriteria::Online:
            hideFriends(searchString, Status::Online);
            hideFriends(QString(), Status::Offline, true);

            hideGroups(searchString);
            break;
        case FilterCriteria::Offline:
            hideFriends(QString(), Status::Online, true);
            hideFriends(searchString, Status::Offline);

            hideGroups(QString(), true);
            break;
        case FilterCriteria::Friends:
            hideFriends(searchString, Status::Online);
            hideFriends(searchString, Status::Offline);

            hideGroups(QString(), true);
            break;
        case FilterCriteria::Groups:
            hideFriends(QString(), Status::Online, true);
            hideFriends(QString(), Status::Offline, true);

            hideGroups(searchString);
            break;
        default:
            return;
    }

    contactListWidget->reDraw();
}

void Widget::hideFriends(QString searchString, Status status, bool hideAll)
{
    QVBoxLayout* friends = contactListWidget->getFriendLayout(status);
    int friendCount = friends->count(), index;

    for (index = 0; index<friendCount; index++)
    {
        FriendWidget* friendWidget = static_cast<FriendWidget*>(friends->itemAt(index)->widget());
        QString friendName = friendWidget->getName();

        if (!friendName.contains(searchString, Qt::CaseInsensitive) || hideAll)
            friendWidget->setVisible(false);
        else
            friendWidget->setVisible(true);
    }
}

void Widget::hideGroups(QString searchString, bool hideAll)
{
    QVBoxLayout* groups = contactListWidget->getGroupLayout();
    int groupCount = groups->count(), index;

    for (index = 0; index<groupCount; index++)
    {
        GroupWidget* groupWidget = static_cast<GroupWidget*>(groups->itemAt(index)->widget());
        QString groupName = groupWidget->getName();

        if (!groupName.contains(searchString, Qt::CaseInsensitive) || hideAll)
            groupWidget->setVisible(false);
        else
            groupWidget->setVisible(true);
    }
}

void Widget::setActiveToolMenuButton(ActiveToolMenuButton newActiveButton)
{
    ui->addButton->setChecked(newActiveButton == Widget::AddButton);
    ui->addButton->setDisabled(newActiveButton == Widget::AddButton);
    ui->groupButton->setChecked(newActiveButton == Widget::GroupButton);
    ui->groupButton->setDisabled(newActiveButton == Widget::GroupButton);
    ui->transferButton->setChecked(newActiveButton == Widget::TransferButton);
    ui->transferButton->setDisabled(newActiveButton == Widget::TransferButton);
    ui->settingsButton->setChecked(newActiveButton == Widget::SettingButton);
    ui->settingsButton->setDisabled(newActiveButton == Widget::SettingButton);
}

void Widget::retranslateUi()
{
    QString name = ui->nameLabel->text(), status = ui->statusLabel->text();
    ui->retranslateUi(this);
    ui->nameLabel->setText(name);
    ui->statusLabel->setText(status);
    ui->searchContactFilterCBox->clear();
    ui->searchContactFilterCBox->addItem(tr("All"));
    ui->searchContactFilterCBox->addItem(tr("Online"));
    ui->searchContactFilterCBox->addItem(tr("Offline"));
    ui->searchContactFilterCBox->addItem(tr("Friends"));
    ui->searchContactFilterCBox->addItem(tr("Groups"));
    ui->searchContactText->setPlaceholderText(tr("Search Contacts"));
    statusOnline->setText(tr("Online", "Button to set your status to 'Online'"));
    statusAway->setText(tr("Away", "Button to set your status to 'Away'"));
    statusBusy->setText(tr("Busy", "Button to set your status to 'Busy'"));
    setWindowTitle(tr("Settings"));
}
