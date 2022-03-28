/*
    Copyright © 2014-2022 by The qTox Project Contributors

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

#include <cassert>

#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QShortcut>
#include <QString>
#include <QSvgRenderer>
#include <QWindow>
#ifdef Q_OS_MAC
#include <QMenuBar>
#include <QSignalMapper>
#include <QWindow>
#endif

#include "audio/audio.h"
#include "circlewidget.h"
#include "contentdialog.h"
#include "contentlayout.h"
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "groupwidget.h"
#include "maskablepixmapwidget.h"
#include "splitterrestorer.h"
#include "form/groupchatform.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/chatlog/documentcache.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/corefile.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/chathistory.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/chatroom/groupchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/model/profile/profileinfo.h"
#include "src/model/status.h"
#include "src/net/updatecheck.h"
#include "src/nexus.h"
#include "src/persistence/offlinemsgengine.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/platform/timer.h"
#include "src/widget/contentdialogmanager.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/chatform.h"
#include "src/widget/form/filesform.h"
#include "src/widget/form/groupinviteform.h"
#include "src/widget/form/profileform.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "src/widget/tool/messageboxmanager.h"
#include "tool/removechatdialog.h"
#include "src/persistence/smileypack.h"
#include "src/persistence/toxsave.h"
#include "src/ipc.h"

namespace {

/**
 * @brief Dangerous way to find out if a path is writable.
 * @param filepath Path to file which should be deleted.
 * @return True, if file writeable, false otherwise.
 */
bool tryRemoveFile(const QString& filepath)
{
    QFile tmp(filepath);
    bool writable = tmp.open(QIODevice::WriteOnly);
    tmp.remove();
    return writable;
}

const QString activateHandlerKey("activate");
const QString saveHandlerKey("save");

} // namespace

bool Widget::toxActivateEventHandler(const QByteArray& data, void* userData)
{
    std::ignore = data;

    qDebug() << "Handling" << activateHandlerKey << "event from other instance";
    static_cast<Widget*>(userData)->forceShow();
    return true;
}

void Widget::acceptFileTransfer(const ToxFile& file, const QString& path)
{
    QString filepath;
    int number = 0;

    QString suffix = QFileInfo(file.fileName).completeSuffix();
    QString base = QFileInfo(file.fileName).baseName();

    do {
        filepath = QString("%1/%2%3.%4")
                       .arg(path, base,
                            number > 0 ? QString(" (%1)").arg(QString::number(number)) : QString(),
                            suffix);
        ++number;
    } while (QFileInfo(filepath).exists());

    // Do not automatically accept the file-transfer if the path is not writable.
    // The user can still accept it manually.
    if (tryRemoveFile(filepath)) {
        CoreFile* coreFile = core->getCoreFile();
        coreFile->acceptFileRecvRequest(file.friendId, file.fileNum, filepath);
    } else {
        qWarning() << "Cannot write to " << filepath;
    }
}

Widget* Widget::instance{nullptr};

Widget::Widget(Profile &profile_, IAudioControl& audio_, CameraSource& cameraSource_,
    Settings& settings_, Style& style_, IPC& ipc_, Nexus& nexus_, QWidget* parent)
    : QMainWindow(parent)
    , profile{profile_}
    , trayMenu{nullptr}
    , ui(new Ui::MainWindow)
    , activeChatroomWidget{nullptr}
    , eventFlag(false)
    , eventIcon(false)
    , audio(audio_)
    , settings(settings_)
#if DESKTOP_NOTIFICATIONS
    , notifier{settings}
#endif
    , smileyPack(new SmileyPack(settings))
    , documentCache(new DocumentCache(*smileyPack, settings))
    , cameraSource{cameraSource_}
    , style{style_}
    , messageBoxManager(new MessageBoxManager(this))
    , friendList(new FriendList())
    , groupList(new GroupList())
    , contentDialogManager(new ContentDialogManager(*friendList))
    , ipc{ipc_}
    , toxSave(new ToxSave{settings, ipc, this})
    , nexus{nexus_}
{
    installEventFilter(this);
    QString locale = settings.getTranslation();
    Translator::translate(locale);
}

void Widget::init()
{
    ui->setupUi(this);

    QIcon themeIcon = QIcon::fromTheme("qtox");
    if (!themeIcon.isNull()) {
        setWindowIcon(themeIcon);
    }

    timer = new QTimer();
    timer->start(1000);

    icon_size = 15;

    actionShow = new QAction(this);
    connect(actionShow, &QAction::triggered, this, &Widget::forceShow);

    // Preparing icons and set their size
    statusOnline = new QAction(this);
    statusOnline->setIcon(
        prepareIcon(Status::getIconPath(Status::Status::Online), icon_size, icon_size));
    connect(statusOnline, &QAction::triggered, this, &Widget::setStatusOnline);

    statusAway = new QAction(this);
    statusAway->setIcon(prepareIcon(Status::getIconPath(Status::Status::Away), icon_size, icon_size));
    connect(statusAway, &QAction::triggered, this, &Widget::setStatusAway);

    statusBusy = new QAction(this);
    statusBusy->setIcon(prepareIcon(Status::getIconPath(Status::Status::Busy), icon_size, icon_size));
    connect(statusBusy, &QAction::triggered, this, &Widget::setStatusBusy);

    actionLogout = new QAction(this);
    actionLogout->setIcon(prepareIcon(":/img/others/logout-icon.svg", icon_size, icon_size));

    actionQuit = new QAction(this);
#ifndef Q_OS_OSX
    actionQuit->setMenuRole(QAction::QuitRole);
#endif

    actionQuit->setIcon(
        prepareIcon(style.getImagePath("rejectCall/rejectCall.svg", settings), icon_size, icon_size));
    connect(actionQuit, &QAction::triggered, qApp, &QApplication::quit);

    layout()->setContentsMargins(0, 0, 0, 0);

    profilePicture = new MaskablePixmapWidget(this, QSize(40, 40), ":/img/avatar_mask.svg");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.svg"));
    profilePicture->setClickable(true);
    profilePicture->setObjectName("selfAvatar");
    ui->myProfile->insertWidget(0, profilePicture);
    ui->myProfile->insertSpacing(1, 7);

    filterMenu = new QMenu(this);
    filterGroup = new QActionGroup(this);
    filterDisplayGroup = new QActionGroup(this);

    filterDisplayName = new QAction(this);
    filterDisplayName->setCheckable(true);
    filterDisplayName->setChecked(true);
    filterDisplayGroup->addAction(filterDisplayName);
    filterMenu->addAction(filterDisplayName);
    filterDisplayActivity = new QAction(this);
    filterDisplayActivity->setCheckable(true);
    filterDisplayGroup->addAction(filterDisplayActivity);
    filterMenu->addAction(filterDisplayActivity);
    settings.getFriendSortingMode() == FriendListWidget::SortingMode::Name
        ? filterDisplayName->setChecked(true)
        : filterDisplayActivity->setChecked(true);
    filterMenu->addSeparator();

    filterAllAction = new QAction(this);
    filterAllAction->setCheckable(true);
    filterAllAction->setChecked(true);
    filterGroup->addAction(filterAllAction);
    filterMenu->addAction(filterAllAction);
    filterOnlineAction = new QAction(this);
    filterOnlineAction->setCheckable(true);
    filterGroup->addAction(filterOnlineAction);
    filterMenu->addAction(filterOnlineAction);
    filterOfflineAction = new QAction(this);
    filterOfflineAction->setCheckable(true);
    filterGroup->addAction(filterOfflineAction);
    filterMenu->addAction(filterOfflineAction);
    filterFriendsAction = new QAction(this);
    filterFriendsAction->setCheckable(true);
    filterGroup->addAction(filterFriendsAction);
    filterMenu->addAction(filterFriendsAction);
    filterGroupsAction = new QAction(this);
    filterGroupsAction->setCheckable(true);
    filterGroup->addAction(filterGroupsAction);
    filterMenu->addAction(filterGroupsAction);

    ui->searchContactFilterBox->setMenu(filterMenu);

    core = &profile.getCore();
    auto coreExt = core->getExt();

    sharedMessageProcessorParams.reset(new MessageProcessor::SharedParams(core->getMaxMessageSize(), coreExt->getMaxExtendedMessageSize()));

    chatListWidget = new FriendListWidget(*core, this, settings, style,
        *messageBoxManager, *friendList, *groupList, profile, settings.getGroupchatPosition());
    connect(chatListWidget, &FriendListWidget::searchCircle, this, &Widget::searchCircle);
    connect(chatListWidget, &FriendListWidget::connectCircleWidget, this,
            &Widget::connectCircleWidget);
    ui->friendList->setWidget(chatListWidget);
    ui->friendList->setLayoutDirection(Qt::RightToLeft);
    ui->friendList->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->statusLabel->setEditable(true);

    QMenu* statusButtonMenu = new QMenu(ui->statusButton);
    statusButtonMenu->addAction(statusOnline);
    statusButtonMenu->addAction(statusAway);
    statusButtonMenu->addAction(statusBusy);
    ui->statusButton->setMenu(statusButtonMenu);

    // disable proportional scaling
    ui->mainSplitter->setStretchFactor(0, 0);
    ui->mainSplitter->setStretchFactor(1, 1);

    onStatusSet(Status::Status::Offline);

    // Disable some widgets until we're connected to the DHT
    ui->statusButton->setEnabled(false);

    style.setThemeColor(settings, settings.getThemeColor());

    CoreFile* coreFile = core->getCoreFile();
    filesForm = new FilesForm(*coreFile, settings, style, *messageBoxManager, *friendList);
    addFriendForm = new AddFriendForm(core->getSelfId(), settings, style,
        *messageBoxManager, *core);
    groupInviteForm = new GroupInviteForm(settings, *core);

#if UPDATE_CHECK_ENABLED
    updateCheck = std::unique_ptr<UpdateCheck>(new UpdateCheck(settings));
    connect(updateCheck.get(), &UpdateCheck::updateAvailable, this, &Widget::onUpdateAvailable);
#endif
    settingsWidget = new SettingsWidget(updateCheck.get(), audio, core, *smileyPack,
        cameraSource, settings, style, *messageBoxManager, profile, this);
#if UPDATE_CHECK_ENABLED
    updateCheck->checkForUpdate();
#endif

    profileInfo = new ProfileInfo(core, &profile, settings, nexus);
    profileForm = new ProfileForm(profileInfo, settings, style, *messageBoxManager);

#if DESKTOP_NOTIFICATIONS
    notificationGenerator.reset(new NotificationGenerator(settings, &profile));
    connect(&notifier, &DesktopNotify::notificationClosed, notificationGenerator.get(), &NotificationGenerator::onNotificationActivated);
#endif

    // connect logout tray menu action
    connect(actionLogout, &QAction::triggered, profileForm, &ProfileForm::onLogoutClicked);

    connect(&profile, &Profile::selfAvatarChanged, profileForm, &ProfileForm::onSelfAvatarLoaded);

    connect(coreFile, &CoreFile::fileReceiveRequested, this, &Widget::onFileReceiveRequested);
    connect(ui->addButton, &QPushButton::clicked, this, &Widget::onAddClicked);
    connect(ui->groupButton, &QPushButton::clicked, this, &Widget::onGroupClicked);
    connect(ui->transferButton, &QPushButton::clicked, this, &Widget::onTransferClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &Widget::onShowSettings);
    connect(profilePicture, &MaskablePixmapWidget::clicked, this, &Widget::showProfile);
    connect(ui->nameLabel, &CroppingLabel::clicked, this, &Widget::showProfile);
    connect(ui->statusLabel, &CroppingLabel::editFinished, this, &Widget::onStatusMessageChanged);
    connect(ui->mainSplitter, &QSplitter::splitterMoved, this, &Widget::onSplitterMoved);
    connect(addFriendForm, &AddFriendForm::friendRequested, this, &Widget::friendRequested);
    connect(groupInviteForm, &GroupInviteForm::groupCreate, core, &Core::createGroup);
    connect(timer, &QTimer::timeout, this, &Widget::onUserAwayCheck);
    connect(timer, &QTimer::timeout, this, &Widget::onEventIconTick);
    connect(timer, &QTimer::timeout, this, &Widget::onTryCreateTrayIcon);
    connect(ui->searchContactText, &QLineEdit::textChanged, this, &Widget::searchChats);
    connect(filterGroup, &QActionGroup::triggered, this, &Widget::searchChats);
    connect(filterDisplayGroup, &QActionGroup::triggered, this, &Widget::changeDisplayMode);
    connect(ui->friendList, &QWidget::customContextMenuRequested, this, &Widget::friendListContextMenu);

    // NOTE: Order of these signals as well as the use of QueuedConnection is important!
    // Qt::AutoConnection, signals emitted from the same thread as Widget will
    // be serviced before other signals. This is a problem when we have tight
    // calls between file control and file info callbacks.
    //
    // File info callbacks are called from the core thread and will use
    // QueuedConnection by default, our control path can easily end up on the
    // same thread as widget. This can result in the following behavior if we
    // are not careful
    //
    // * File data is received
    // * User presses pause at the same time
    // * Pause waits for data receive callback to complete (and emit fileTransferInfo)
    // * Pause is executed and emits fileTransferPaused
    // * Pause signal is handled by Qt::DirectConnection
    // * fileTransferInfo signal is handled after by Qt::QueuedConnection
    //
    // This results in stale file state! In these conditions if we are not
    // careful toxcore will think we are paused but our UI will think we are
    // resumed, because the last signal they got was a transmitting file info
    // signal!
    connect(coreFile, &CoreFile::fileTransferInfo, this, &Widget::dispatchFile, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileSendStarted, this, &Widget::dispatchFile, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileReceiveRequested, this, &Widget::dispatchFile, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileTransferAccepted, this, &Widget::dispatchFile, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileTransferCancelled, this, &Widget::dispatchFile, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileTransferFinished, this, &Widget::dispatchFile, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileTransferPaused, this, &Widget::dispatchFile, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileTransferRemotePausedUnpaused, this, &Widget::dispatchFileWithBool, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileTransferBrokenUnbroken, this, &Widget::dispatchFileWithBool, Qt::QueuedConnection);
    connect(coreFile, &CoreFile::fileSendFailed, this, &Widget::dispatchFileSendFailed, Qt::QueuedConnection);
    // NOTE: We intentionally do not connect the fileUploadFinished and fileDownloadFinished signals
    // because they are duplicates of fileTransferFinished NOTE: We don't hook up the
    // fileNameChanged signal since it is only emitted before a fileReceiveRequest. We get the
    // initial request with the sanitized name so there is no work for us to do

    // keyboard shortcuts
    auto* const quitShortcut = new QShortcut(Qt::CTRL + Qt::Key_Q, this);
    connect(quitShortcut, &QShortcut::activated, qApp, &QApplication::quit);
    new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this, SLOT(previousChat()));
    new QShortcut(Qt::CTRL + Qt::Key_Tab, this, SLOT(nextChat()));
    new QShortcut(Qt::CTRL + Qt::Key_PageUp, this, SLOT(previousChat()));
    new QShortcut(Qt::CTRL + Qt::Key_PageDown, this, SLOT(nextChat()));
    new QShortcut(Qt::Key_F11, this, SLOT(toggleFullscreen()));

#ifdef Q_OS_MAC
    QMenuBar* globalMenu = nexus.globalMenuBar;
    QAction* windowMenu = nexus.windowMenu->menuAction();
    QAction* viewMenu = nexus.viewMenu->menuAction();
    QAction* frontAction = nexus.frontAction;

    fileMenu = globalMenu->insertMenu(viewMenu, new QMenu(this));

    editProfileAction = fileMenu->menu()->addAction(QString());
    connect(editProfileAction, &QAction::triggered, this, &Widget::showProfile);

    changeStatusMenu = fileMenu->menu()->addMenu(QString());
    fileMenu->menu()->addAction(changeStatusMenu->menuAction());
    changeStatusMenu->addAction(statusOnline);
    changeStatusMenu->addSeparator();
    changeStatusMenu->addAction(statusAway);
    changeStatusMenu->addAction(statusBusy);

    fileMenu->menu()->addSeparator();
    logoutAction = fileMenu->menu()->addAction(QString());
    connect(logoutAction, &QAction::triggered, [this]() { nexus.showLogin(); });

    editMenu = globalMenu->insertMenu(viewMenu, new QMenu(this));
    editMenu->menu()->addSeparator();

    viewMenu->menu()->insertMenu(nexus.fullscreenAction, filterMenu);

    viewMenu->menu()->insertSeparator(nexus.fullscreenAction);

    contactMenu = globalMenu->insertMenu(windowMenu, new QMenu(this));

    addContactAction = contactMenu->menu()->addAction(QString());
    connect(addContactAction, &QAction::triggered, this, &Widget::onAddClicked);

    nextConversationAction = new QAction(this);
    nexus.windowMenu->insertAction(frontAction, nextConversationAction);
    nextConversationAction->setShortcut(QKeySequence::SelectNextPage);
    connect(nextConversationAction, &QAction::triggered, [this]() {
        if (contentDialogManager->current() == QApplication::activeWindow())
            contentDialogManager->current()->cycleChats(true);
        else if (QApplication::activeWindow() == this)
            cycleChats(true);
    });

    previousConversationAction = new QAction(this);
    nexus.windowMenu->insertAction(frontAction, previousConversationAction);
    previousConversationAction->setShortcut(QKeySequence::SelectPreviousPage);
    connect(previousConversationAction, &QAction::triggered, [this] {
        if (contentDialogManager->current() == QApplication::activeWindow())
            contentDialogManager->current()->cycleChats(false);
        else if (QApplication::activeWindow() == this)
            cycleChats(false);
    });

    windowMenu->menu()->insertSeparator(frontAction);

    QAction* preferencesAction = viewMenu->menu()->addAction(QString());
    preferencesAction->setMenuRole(QAction::PreferencesRole);
    connect(preferencesAction, &QAction::triggered, this, &Widget::onShowSettings);

    QAction* aboutAction = viewMenu->menu()->addAction(QString());
    aboutAction->setMenuRole(QAction::AboutRole);
    connect(aboutAction, &QAction::triggered, [this]() {
        onShowSettings();
        settingsWidget->showAbout();
    });

    QMenu* dockChangeStatusMenu = new QMenu(tr("Status"), this);
    dockChangeStatusMenu->addAction(statusOnline);
    statusOnline->setIconVisibleInMenu(true);
    dockChangeStatusMenu->addSeparator();
    dockChangeStatusMenu->addAction(statusAway);
    dockChangeStatusMenu->addAction(statusBusy);
    nexus.dockMenu->addAction(dockChangeStatusMenu->menuAction());

    connect(this, &Widget::windowStateChanged, &nexus, &Nexus::onWindowStateChanged);
#endif

    contentLayout = nullptr;
    onSeparateWindowChanged(settings.getSeparateWindow(), false);

    ui->addButton->setCheckable(true);
    ui->groupButton->setCheckable(true);
    ui->transferButton->setCheckable(true);
    ui->settingsButton->setCheckable(true);

    if (contentLayout) {
        onAddClicked();
    }

    // restore window state
    restoreGeometry(settings.getWindowGeometry());
    restoreState(settings.getWindowState());
    SplitterRestorer restorer(ui->mainSplitter);
    restorer.restore(settings.getSplitterState(), size());

    friendRequestsButton = nullptr;
    groupInvitesButton = nullptr;
    unreadGroupInvites = 0;

    connect(addFriendForm, &AddFriendForm::friendRequested, this, &Widget::friendRequestsUpdate);
    connect(addFriendForm, &AddFriendForm::friendRequestsSeen, this, &Widget::friendRequestsUpdate);
    connect(addFriendForm, &AddFriendForm::friendRequestAccepted, this, &Widget::friendRequestAccepted);
    connect(groupInviteForm, &GroupInviteForm::groupInvitesSeen, this, &Widget::groupInvitesClear);
    connect(groupInviteForm, &GroupInviteForm::groupInviteAccepted, this,
            &Widget::onGroupInviteAccepted);

    // settings
    connect(&settings, &Settings::showSystemTrayChanged, this, &Widget::onSetShowSystemTray);
    connect(&settings, &Settings::separateWindowChanged, this, &Widget::onSeparateWindowClicked);
    connect(&settings, &Settings::compactLayoutChanged, chatListWidget,
            &FriendListWidget::onCompactChanged);
    connect(&settings, &Settings::groupchatPositionChanged, chatListWidget,
            &FriendListWidget::onGroupchatPositionChanged);

    connect(&style, &Style::themeReload, this, &Widget::reloadTheme);

    reloadTheme();
    updateIcons();
    retranslateUi();
    Translator::registerHandler(std::bind(&Widget::retranslateUi, this), this);

    if (!settings.getShowSystemTray()) {
        show();
    }

#ifdef Q_OS_MAC
    nexus.updateWindows();
#endif
}

bool Widget::eventFilter(QObject* obj, QEvent* event)
{
    QWindowStateChangeEvent* ce = nullptr;
    Qt::WindowStates state = windowState();

    switch (event->type()) {
    case QEvent::Close:
        // It's needed if user enable `Close to tray`
        wasMaximized = state.testFlag(Qt::WindowMaximized);
        break;

    case QEvent::WindowStateChange:
        ce = static_cast<QWindowStateChangeEvent*>(event);
        if (state.testFlag(Qt::WindowMinimized) && obj) {
            wasMaximized = ce->oldState().testFlag(Qt::WindowMaximized);
        }

#ifdef Q_OS_MAC
        emit windowStateChanged(windowState());
#endif
        break;
    default:
        break;
    }

    return false;
}

void Widget::updateIcons()
{
    if (!icon) {
        return;
    }

    const QString assetSuffix = Status::getAssetSuffix(static_cast<Status::Status>(
                                    ui->statusButton->property("status").toInt()))
                                + (eventIcon ? QStringLiteral("_event") : QString());

    // Some builds of Qt appear to have a bug in icon loading:
    // QIcon::hasThemeIcon is sometimes unaware that the icon returned
    // from QIcon::fromTheme was a fallback icon, causing hasThemeIcon to
    // incorrectly return true.
    //
    // In qTox this leads to the tray and window icons using the static qTox logo
    // icon instead of an icon based on the current presence status.
    //
    // This workaround checks for an icon that definitely does not exist to
    // determine if hasThemeIcon can be trusted.
    //
    // On systems with the Qt bug, this workaround will always use our included
    // icons but user themes will be unable to override them.
    static bool checkedHasThemeIcon = false;
    static bool hasThemeIconBug = false;

    if (!checkedHasThemeIcon) {
        hasThemeIconBug = QIcon::hasThemeIcon("qtox-asjkdfhawjkeghdfjgh");
        checkedHasThemeIcon = true;

        if (hasThemeIconBug) {
            qDebug()
                << "Detected buggy QIcon::hasThemeIcon. Icon overrides from theme will be ignored.";
        }
    }

    QIcon ico;
    if (!hasThemeIconBug && QIcon::hasThemeIcon("qtox-" + assetSuffix)) {
        ico = QIcon::fromTheme("qtox-" + assetSuffix);
    } else {
        QString color = settings.getLightTrayIcon() ? QStringLiteral("light") : QStringLiteral("dark");
        QString path = ":/img/taskbar/" + color + "/taskbar_" + assetSuffix + ".svg";
        QSvgRenderer renderer(path);

        // Prepare a QImage with desired characteritisc
        QImage image = QImage(250, 250, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        renderer.render(&painter);
        ico = QIcon(QPixmap::fromImage(image));
    }

    setWindowIcon(ico);
    if (icon) {
        icon->setIcon(ico);
    }
}

Widget::~Widget()
{
    ipc.unregisterEventHandler(activateHandlerKey);

    QWidgetList windowList = QApplication::topLevelWidgets();

    for (QWidget* window : windowList) {
        if (window != this) {
            window->close();
        }
    }

    Translator::unregister(this);
    if (icon) {
        icon->hide();
    }

    for (Group* g : groupList->getAllGroups()) {
        removeGroup(g, true);
    }

    for (Friend* f : friendList->getAllFriends()) {
        removeFriend(f, true);
    }

    for (auto form : chatForms) {
        delete form;
    }

    delete profileForm;
    delete profileInfo;
    delete addFriendForm;
    delete groupInviteForm;
    delete filesForm;
    delete timer;
    delete contentLayout;
    delete settingsWidget;

    friendList->clear();
    groupList->clear();
    delete trayMenu;
    delete ui;
    instance = nullptr;
}

/**
 * @brief Switches to the About settings page.
 */
void Widget::showUpdateDownloadProgress()
{
    onShowSettings();
    settingsWidget->showAbout();
}

void Widget::moveEvent(QMoveEvent* event)
{
    if (event->type() == QEvent::Move) {
        saveWindowGeometry();
        saveSplitterGeometry();
    }

    QWidget::moveEvent(event);
}

void Widget::closeEvent(QCloseEvent* event)
{
    if (settings.getShowSystemTray() && settings.getCloseToTray()) {
        QWidget::closeEvent(event);
    } else {
        if (autoAwayActive) {
            emit statusSet(Status::Status::Online);
            autoAwayActive = false;
        }
        saveWindowGeometry();
        saveSplitterGeometry();
        QWidget::closeEvent(event);
        qApp->quit();
    }
}

void Widget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() && settings.getShowSystemTray() && settings.getMinimizeToTray()) {
            hide();
        }
    }
}

void Widget::resizeEvent(QResizeEvent* event)
{
    saveWindowGeometry();
    QMainWindow::resizeEvent(event);
}

QString Widget::getUsername()
{
    return core->getUsername();
}

void Widget::onSelfAvatarLoaded(const QPixmap& pic)
{
    profilePicture->setPixmap(pic);
}

void Widget::onCoreChanged(Core& core_)
{
    core = &core_;
    connect(core, &Core::connected, this, &Widget::onConnected);
    connect(core, &Core::disconnected, this, &Widget::onDisconnected);
    connect(core, &Core::statusSet, this, &Widget::onStatusSet);
    connect(core, &Core::usernameSet, this, &Widget::setUsername);
    connect(core, &Core::statusMessageSet, this, &Widget::setStatusMessage);
    connect(core, &Core::friendAdded, this, &Widget::addFriend);
    connect(core, &Core::failedToAddFriend, this, &Widget::addFriendFailed);
    connect(core, &Core::friendUsernameChanged, this, &Widget::onFriendUsernameChanged);
    connect(core, &Core::friendStatusChanged, this, &Widget::onCoreFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, this, &Widget::onFriendStatusMessageChanged);
    connect(core, &Core::friendRequestReceived, this, &Widget::onFriendRequestReceived);
    connect(core, &Core::friendMessageReceived, this, &Widget::onFriendMessageReceived);
    connect(core, &Core::receiptRecieved, this, &Widget::onReceiptReceived);
    connect(core, &Core::groupInviteReceived, this, &Widget::onGroupInviteReceived);
    connect(core, &Core::groupMessageReceived, this, &Widget::onGroupMessageReceived);
    connect(core, &Core::groupPeerlistChanged, this, &Widget::onGroupPeerlistChanged);
    connect(core, &Core::groupPeerNameChanged, this, &Widget::onGroupPeerNameChanged);
    connect(core, &Core::groupTitleChanged, this, &Widget::onGroupTitleChanged);
    connect(core, &Core::groupPeerAudioPlaying, this, &Widget::onGroupPeerAudioPlaying);
    connect(core, &Core::emptyGroupCreated, this, &Widget::onEmptyGroupCreated);
    connect(core, &Core::groupJoined, this, &Widget::onGroupJoined);
    connect(core, &Core::friendTypingChanged, this, &Widget::onFriendTypingChanged);
    connect(core, &Core::groupSentFailed, this, &Widget::onGroupSendFailed);
    connect(core, &Core::usernameSet, this, &Widget::refreshPeerListsLocal);

    auto coreExt = core->getExt();

    connect(coreExt, &CoreExt::extendedMessageReceived, this, &Widget::onFriendExtMessageReceived);
    connect(coreExt, &CoreExt::extendedReceiptReceived, this, &Widget::onExtReceiptReceived);
    connect(coreExt, &CoreExt::extendedMessageSupport, this, &Widget::onExtendedMessageSupport);

    connect(this, &Widget::statusSet, core, &Core::setStatus);
    connect(this, &Widget::friendRequested, core, &Core::requestFriendship);
    connect(this, &Widget::friendRequestAccepted, core, &Core::acceptFriendRequest);
    connect(this, &Widget::changeGroupTitle, core, &Core::changeGroupTitle);

    sharedMessageProcessorParams->setPublicKey(core->getSelfPublicKey().toString());
}

void Widget::onConnected()
{
    ui->statusButton->setEnabled(true);
    emit core->statusSet(core->getStatus());
}

void Widget::onDisconnected()
{
    ui->statusButton->setEnabled(false);
    emit core->statusSet(Status::Status::Offline);
}

void Widget::onFailedToStartCore()
{
    QMessageBox critical(this);
    critical.setText(tr(
        "Toxcore failed to start, the application will terminate after you close this message."));
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    qApp->exit(EXIT_FAILURE);
}

void Widget::onBadProxyCore()
{
    settings.setProxyType(Settings::ProxyType::ptNone);
    QMessageBox critical(this);
    critical.setText(tr("Toxcore failed to start with your proxy settings. "
                        "qTox cannot run; please modify your "
                        "settings and restart.",
                        "popup text"));
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    onShowSettings();
}

void Widget::onStatusSet(Status::Status status)
{
    ui->statusButton->setProperty("status", static_cast<int>(status));
    ui->statusButton->setIcon(prepareIcon(getIconPath(status), icon_size, icon_size));
    updateIcons();
}

void Widget::onSeparateWindowClicked(bool separate)
{
    onSeparateWindowChanged(separate, true);
}

void Widget::onSeparateWindowChanged(bool separate, bool clicked)
{
    if (!separate) {
        QWindowList windowList = QGuiApplication::topLevelWindows();

        for (QWindow* window : windowList) {
            if (window->objectName() == "detachedWindow") {
                window->close();
            }
        }

        QWidget* contentWidget = new QWidget(this);
        contentWidget->setObjectName("contentWidget");

        contentLayout = new ContentLayout(settings, style, contentWidget);
        ui->mainSplitter->addWidget(contentWidget);

        setMinimumWidth(775);

        SplitterRestorer restorer(ui->mainSplitter);
        restorer.restore(settings.getSplitterState(), size());

        onShowSettings();
    } else {
        int width = ui->friendList->size().width();
        QSize size;
        QPoint pos;

        if (contentLayout) {
            pos = mapToGlobal(ui->mainSplitter->widget(1)->pos());
            size = ui->mainSplitter->widget(1)->size();
        }

        if (contentLayout) {
            contentLayout->clear();
            contentLayout->parentWidget()->setParent(nullptr); // Remove from splitter.
            contentLayout->parentWidget()->hide();
            contentLayout->parentWidget()->deleteLater();
            contentLayout->deleteLater();
            contentLayout = nullptr;
        }

        setMinimumWidth(ui->tooliconsZone->sizeHint().width());

        if (clicked) {
            showNormal();
            resize(width, height());

            if (settingsWidget) {
                ContentLayout* contentLayout_ = createContentDialog((DialogType::SettingDialog));
                contentLayout_->parentWidget()->resize(size);
                contentLayout_->parentWidget()->move(pos);
                settingsWidget->show(contentLayout_);
                setActiveToolMenuButton(ActiveToolMenuButton::None);
            }
        }

        setWindowTitle(QString());
        setActiveToolMenuButton(ActiveToolMenuButton::None);
    }
}

void Widget::setWindowTitle(const QString& title)
{
    if (title.isEmpty()) {
        QMainWindow::setWindowTitle(QApplication::applicationName());
    } else {
        QString tmp = title;
        /// <[^>]*> Regexp to remove HTML tags, in case someone used them in title
        QMainWindow::setWindowTitle(tmp.remove(QRegExp("<[^>]*>")) + QStringLiteral(" - ")
                                    + QApplication::applicationName());
    }
}

void Widget::forceShow()
{
    hide(); // Workaround to force minimized window to be restored
    show();
    activateWindow();
}

void Widget::onAddClicked()
{
    if (settings.getSeparateWindow()) {
        if (!addFriendForm->isShown()) {
            addFriendForm->show(createContentDialog(DialogType::AddDialog));
        }

        setActiveToolMenuButton(ActiveToolMenuButton::None);
    } else {
        hideMainForms(nullptr);
        addFriendForm->show(contentLayout);
        setWindowTitle(fromDialogType(DialogType::AddDialog));
        setActiveToolMenuButton(ActiveToolMenuButton::AddButton);
    }
}

void Widget::onGroupClicked()
{
    if (settings.getSeparateWindow()) {
        if (!groupInviteForm->isShown()) {
            groupInviteForm->show(createContentDialog(DialogType::GroupDialog));
        }

        setActiveToolMenuButton(ActiveToolMenuButton::None);
    } else {
        hideMainForms(nullptr);
        groupInviteForm->show(contentLayout);
        setWindowTitle(fromDialogType(DialogType::GroupDialog));
        setActiveToolMenuButton(ActiveToolMenuButton::GroupButton);
    }
}

void Widget::onTransferClicked()
{
    if (settings.getSeparateWindow()) {
        if (!filesForm->isShown()) {
            filesForm->show(createContentDialog(DialogType::TransferDialog));
        }

        setActiveToolMenuButton(ActiveToolMenuButton::None);
    } else {
        hideMainForms(nullptr);
        filesForm->show(contentLayout);
        setWindowTitle(fromDialogType(DialogType::TransferDialog));
        setActiveToolMenuButton(ActiveToolMenuButton::TransferButton);
    }
}

void Widget::onIconClick(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (isHidden() || isMinimized()) {
            if (wasMaximized) {
                showMaximized();
            } else {
                showNormal();
            }

            activateWindow();
        } else if (!isActiveWindow()) {
            activateWindow();
        } else {
            wasMaximized = isMaximized();
            hide();
        }
    } else if (reason == QSystemTrayIcon::Unknown) {
        if (isHidden()) {
            forceShow();
        }
    }
}

void Widget::onShowSettings()
{
    if (settings.getSeparateWindow()) {
        if (!settingsWidget->isShown()) {
            settingsWidget->show(createContentDialog(DialogType::SettingDialog));
        }

        setActiveToolMenuButton(ActiveToolMenuButton::None);
    } else {
        hideMainForms(nullptr);
        settingsWidget->show(contentLayout);
        setWindowTitle(fromDialogType(DialogType::SettingDialog));
        setActiveToolMenuButton(ActiveToolMenuButton::SettingButton);
    }
}

void Widget::showProfile() // onAvatarClicked, onUsernameClicked
{
    if (settings.getSeparateWindow()) {
        if (!profileForm->isShown()) {
            profileForm->show(createContentDialog(DialogType::ProfileDialog));
        }

        setActiveToolMenuButton(ActiveToolMenuButton::None);
    } else {
        hideMainForms(nullptr);
        profileForm->show(contentLayout);
        setWindowTitle(fromDialogType(DialogType::ProfileDialog));
        setActiveToolMenuButton(ActiveToolMenuButton::None);
    }
}

void Widget::hideMainForms(GenericChatroomWidget* chatroomWidget)
{
    setActiveToolMenuButton(ActiveToolMenuButton::None);

    if (contentLayout != nullptr) {
        contentLayout->clear();
    }

    if (activeChatroomWidget != nullptr) {
        activeChatroomWidget->setAsInactiveChatroom();
    }

    activeChatroomWidget = chatroomWidget;
}

void Widget::setUsername(const QString& username)
{
    if (username.isEmpty()) {
        ui->nameLabel->setText(tr("Your name"));
        ui->nameLabel->setToolTip(tr("Your name"));
    } else {
        ui->nameLabel->setText(username);
        ui->nameLabel->setToolTip(
            Qt::convertFromPlainText(username, Qt::WhiteSpaceNormal)); // for overlength names
    }

    sharedMessageProcessorParams->onUserNameSet(username);
}

void Widget::onStatusMessageChanged(const QString& newStatusMessage)
{
    // Keep old status message until Core tells us to set it.
    core->setStatusMessage(newStatusMessage);
}

void Widget::setStatusMessage(const QString& statusMessage)
{
    ui->statusLabel->setText(statusMessage);
    // escape HTML from tooltips and preserve newlines
    // TODO: move newspace preservance to a generic function
    ui->statusLabel->setToolTip("<p style='white-space:pre'>" + statusMessage.toHtmlEscaped() + "</p>");
}

/**
 * @brief Plays a sound via the audioNotification AudioSink
 * @param sound Sound to play
 * @param loop if true, loop the sound until onStopNotification() is called
 */
void Widget::playNotificationSound(IAudioSink::Sound sound, bool loop)
{
    if (!settings.getAudioOutDevEnabled()) {
        // don't try to play sounds if audio is disabled
        return;
    }

    if (audioNotification == nullptr) {
        audioNotification = std::unique_ptr<IAudioSink>(audio.makeSink());
        if (audioNotification == nullptr) {
            qDebug() << "Failed to allocate AudioSink";
            return;
        }
    }

    audioNotification->connectTo_finishedPlaying(this, [this](){ cleanupNotificationSound(); });

    audioNotification->playMono16Sound(sound);

    if (loop) {
        audioNotification->startLoop();
    }
}

void Widget::cleanupNotificationSound()
{
    audioNotification.reset();
}

void Widget::incomingNotification(uint32_t friendNum)
{
    const auto& friendId = friendList->id2Key(friendNum);
    newFriendMessageAlert(friendId, {}, false);

    // loop until call answered or rejected
    playNotificationSound(IAudioSink::Sound::IncomingCall, true);
}

void Widget::outgoingNotification()
{
    // loop until call answered or rejected
    playNotificationSound(IAudioSink::Sound::OutgoingCall, true);
}

void Widget::onCallEnd()
{
    playNotificationSound(IAudioSink::Sound::CallEnd);
}

/**
 * @brief Widget::onStopNotification Stop the notification sound.
 */
void Widget::onStopNotification()
{
    audioNotification.reset();
}

/**
 * @brief Dispatches file to the appropriate chatlog and accepts the transfer if necessary
 */
void Widget::dispatchFile(ToxFile file)
{
    const auto& friendId = friendList->id2Key(file.friendId);
    Friend* f = friendList->findFriend(friendId);
    if (!f) {
        return;
    }

    auto pk = f->getPublicKey();

    if (file.status == ToxFile::INITIALIZING && file.direction == ToxFile::RECEIVING) {
        auto sender =
            (file.direction == ToxFile::SENDING) ? core->getSelfPublicKey() : pk;

        QString autoAcceptDir = settings.getAutoAcceptDir(f->getPublicKey());

        if (autoAcceptDir.isEmpty() && settings.getAutoSaveEnabled()) {
            autoAcceptDir = settings.getGlobalAutoAcceptDir();
        }

        auto maxAutoAcceptSize = settings.getMaxAutoAcceptSize();
        bool autoAcceptSizeCheckPassed =
            maxAutoAcceptSize == 0 || maxAutoAcceptSize >= file.progress.getFileSize();

        if (!autoAcceptDir.isEmpty() && autoAcceptSizeCheckPassed) {
            acceptFileTransfer(file, autoAcceptDir);
        }
    }

    const auto senderPk = (file.direction == ToxFile::SENDING) ? core->getSelfPublicKey() : pk;
    friendChatLogs[pk]->onFileUpdated(senderPk, file);

    filesForm->onFileUpdated(file);
}

void Widget::dispatchFileWithBool(ToxFile file, bool pausedOrBroken)
{
    std::ignore = pausedOrBroken;
    dispatchFile(file);
}

void Widget::dispatchFileSendFailed(uint32_t friendId, const QString& fileName)
{
    const auto& friendPk = friendList->id2Key(friendId);

    auto chatForm = chatForms.find(friendPk);
    if (chatForm == chatForms.end()) {
        return;
    }

    chatForm.value()->addSystemInfoMessage(QDateTime::currentDateTime(),
                                           SystemMessageType::fileSendFailed, {fileName});
}

void Widget::onRejectCall(uint32_t friendId)
{
    CoreAV* const av = core->getAv();
    av->cancelCall(friendId);
}

void Widget::addFriend(uint32_t friendId, const ToxPk& friendPk)
{
    assert(core != nullptr);
    settings.updateFriendAddress(friendPk.toString());

    Friend* newfriend = friendList->addFriend(friendId, friendPk, settings);
    auto rawChatroom = new FriendChatroom(newfriend, contentDialogManager.get(), *core, settings, *groupList);
    std::shared_ptr<FriendChatroom> chatroom(rawChatroom);
    const auto compact = settings.getCompactLayout();
    auto widget = new FriendWidget(chatroom, compact, settings, style, *messageBoxManager, profile);
    connectFriendWidget(*widget);
    auto history = profile.getHistory();

    auto messageProcessor = MessageProcessor(*sharedMessageProcessorParams);
    auto friendMessageDispatcher =
        std::make_shared<FriendMessageDispatcher>(*newfriend, std::move(messageProcessor), *core, *core->getExt());

    // Note: We do not have to connect the message dispatcher signals since
    // ChatHistory hooks them up in a very specific order
    auto chatHistory =
        std::make_shared<ChatHistory>(*newfriend, history, *core, settings,
                                      *friendMessageDispatcher, *friendList,
                                      *groupList);
    auto friendForm = new ChatForm(profile, newfriend, *chatHistory,
        *friendMessageDispatcher, *documentCache, *smileyPack, cameraSource,
        settings, style, *messageBoxManager, *contentDialogManager, *friendList,
        *groupList);
    connect(friendForm, &ChatForm::updateFriendActivity, this, &Widget::updateFriendActivity);

    friendMessageDispatchers[friendPk] = friendMessageDispatcher;
    friendChatLogs[friendPk] = chatHistory;
    friendChatrooms[friendPk] = chatroom;
    friendWidgets[friendPk] = widget;
    chatForms[friendPk] = friendForm;

    const auto activityTime = settings.getFriendActivity(friendPk);
    const auto chatTime = friendForm->getLatestTime();
    if (chatTime > activityTime && chatTime.isValid()) {
        settings.setFriendActivity(friendPk, chatTime);
    }

    chatListWidget->addFriendWidget(widget);


    auto notifyReceivedCallback = [this, friendPk](const ToxPk& author, const Message& message) {
        std::ignore = author;
        newFriendMessageAlert(friendPk, message.content);
    };

    auto notifyReceivedConnection =
        connect(friendMessageDispatcher.get(), &IMessageDispatcher::messageReceived,
                notifyReceivedCallback);

    friendAlertConnections.insert(friendPk, notifyReceivedConnection);
    connect(newfriend, &Friend::aliasChanged, this, &Widget::onFriendAliasChanged);
    connect(newfriend, &Friend::displayedNameChanged, this, &Widget::onFriendDisplayedNameChanged);
    connect(newfriend, &Friend::statusChanged, this, &Widget::onFriendStatusChanged);

    connect(friendForm, &ChatForm::incomingNotification, this, &Widget::incomingNotification);
    connect(friendForm, &ChatForm::outgoingNotification, this, &Widget::outgoingNotification);
    connect(friendForm, &ChatForm::stopNotification, this, &Widget::onStopNotification);
    connect(friendForm, &ChatForm::endCallNotification, this, &Widget::onCallEnd);
    connect(friendForm, &ChatForm::rejectCall, this, &Widget::onRejectCall);

    connect(widget, &FriendWidget::newWindowOpened, this, &Widget::openNewDialog);
    connect(widget, &FriendWidget::chatroomWidgetClicked, this, &Widget::onChatroomWidgetClicked);
    connect(widget, &FriendWidget::chatroomWidgetClicked, friendForm, &ChatForm::focusInput);
    connect(widget, &FriendWidget::friendHistoryRemoved, friendForm, &ChatForm::clearChatArea);
    connect(widget, &FriendWidget::copyFriendIdToClipboard, this, &Widget::copyFriendIdToClipboard);
    connect(widget, &FriendWidget::contextMenuCalled, widget, &FriendWidget::onContextMenuCalled);
    connect(widget, SIGNAL(removeFriend(const ToxPk&)), this, SLOT(removeFriend(const ToxPk&)));

    connect(&profile, &Profile::friendAvatarSet, widget, &FriendWidget::onAvatarSet);
    connect(&profile, &Profile::friendAvatarRemoved, widget, &FriendWidget::onAvatarRemoved);

    // Try to get the avatar from the cache
    QPixmap avatar = profile.loadAvatar(friendPk);
    if (!avatar.isNull()) {
        friendForm->onAvatarChanged(friendPk, avatar);
        widget->onAvatarSet(friendPk, avatar);
    }
}

void Widget::addFriendFailed(const ToxPk& userId, const QString& errorInfo)
{
    std::ignore = userId;
    QString info = QString(tr("Couldn't send friend request"));
    if (!errorInfo.isEmpty()) {
        info = info + QStringLiteral(": ") + errorInfo;
    }

    QMessageBox::critical(nullptr, "Error", info);
}

void Widget::onCoreFriendStatusChanged(int friendId, Status::Status status)
{
    const auto& friendPk = friendList->id2Key(friendId);
    Friend* f = friendList->findFriend(friendPk);
    if (!f) {
        return;
    }

    auto const oldStatus = f->getStatus();
    f->setStatus(status);
    auto const newStatus = f->getStatus();

    auto const startedNegotiating = (newStatus == Status::Status::Negotiating && oldStatus != newStatus);
    if (startedNegotiating) {
        constexpr auto negotiationTimeoutMs = 1000;
        auto negotiateTimer = std::unique_ptr<QTimer>(new QTimer);
        negotiateTimer->setSingleShot(true);
        negotiateTimer->setInterval(negotiationTimeoutMs);
        connect(negotiateTimer.get(), &QTimer::timeout, f, &Friend::onNegotiationComplete);
        negotiateTimer->start();
        negotiateTimers[friendPk] = std::move(negotiateTimer);
    }

    // Any widget behavior will be triggered based off of the status
    // transformations done by the Friend class
}

void Widget::onFriendStatusChanged(const ToxPk& friendPk, Status::Status status)
{
    FriendWidget* widget = friendWidgets[friendPk];

    if (Status::isOnline(status)) {
        chatListWidget->moveWidget(widget, Status::Status::Online);
    } else {
        chatListWidget->moveWidget(widget, Status::Status::Offline);
    }

    widget->updateStatusLight();
    if (widget->isActive()) {
        setWindowTitle(widget->getTitle());
    }

    contentDialogManager->updateFriendStatus(friendPk);
}

void Widget::onFriendStatusMessageChanged(int friendId, const QString& message)
{
    const auto& friendPk = friendList->id2Key(friendId);
    Friend* f = friendList->findFriend(friendPk);
    if (!f) {
        return;
    }

    QString str = message;
    str.replace('\n', ' ').remove('\r').remove(QChar('\0'));
    f->setStatusMessage(str);

    friendWidgets[friendPk]->setStatusMsg(str);
    chatForms[friendPk]->setStatusMessage(str);
}

void Widget::onFriendDisplayedNameChanged(const QString& displayed)
{
    Friend* f = qobject_cast<Friend*>(sender());
    const auto& friendPk = f->getPublicKey();
    for (Group* g : groupList->getAllGroups()) {
        if (g->getPeerList().contains(friendPk)) {
            g->updateUsername(friendPk, displayed);
        }
    }

    FriendWidget* friendWidget = friendWidgets[f->getPublicKey()];
    if (friendWidget->isActive()) {
        formatWindowTitle(displayed);
    }

    chatListWidget->itemsChanged();
}

void Widget::onFriendUsernameChanged(int friendId, const QString& username)
{
    const auto& friendPk = friendList->id2Key(friendId);
    Friend* f = friendList->findFriend(friendPk);
    if (!f) {
        return;
    }

    QString str = username;
    str.replace('\n', ' ').remove('\r').remove(QChar('\0'));
    f->setName(str);
}

void Widget::onFriendAliasChanged(const ToxPk& friendId, const QString& alias)
{
    settings.setFriendAlias(friendId, alias);
    settings.savePersonal();
}

void Widget::onChatroomWidgetClicked(GenericChatroomWidget* widget)
{
    openDialog(widget, /* newWindow = */ false);
}

void Widget::openNewDialog(GenericChatroomWidget* widget)
{
    openDialog(widget, /* newWindow = */ true);
}

void Widget::openDialog(GenericChatroomWidget* widget, bool newWindow)
{
    widget->resetEventFlags();
    widget->updateStatusLight();

    GenericChatForm* form;
    const Friend* frnd = widget->getFriend();
    const Group* group = widget->getGroup();
    bool chatFormIsSet;
    if (frnd) {
        form = chatForms[frnd->getPublicKey()];
        contentDialogManager->focusChat(frnd->getPersistentId());
        chatFormIsSet = contentDialogManager->chatWidgetExists(frnd->getPersistentId());
    } else {
        form = groupChatForms[group->getPersistentId()].data();
        contentDialogManager->focusChat(group->getPersistentId());
        chatFormIsSet = contentDialogManager->chatWidgetExists(group->getPersistentId());
    }

    if ((chatFormIsSet || form->isVisible()) && !newWindow) {
        return;
    }

    if (settings.getSeparateWindow() || newWindow) {
        ContentDialog* dialog = nullptr;

        if (!settings.getDontGroupWindows() && !newWindow) {
            dialog = contentDialogManager->current();
        }

        if (dialog == nullptr) {
            dialog = createContentDialog();
        }

        dialog->show();

        if (frnd) {
            addFriendDialog(frnd, dialog);
        } else {
            addGroupDialog(group, dialog);
        }

        dialog->raise();
        dialog->activateWindow();
    } else {
        hideMainForms(widget);
        if (frnd) {
            chatForms[frnd->getPublicKey()]->show(contentLayout);
        } else {
            groupChatForms[group->getPersistentId()]->show(contentLayout);
        }
        widget->setAsActiveChatroom();
        setWindowTitle(widget->getTitle());
    }
}

void Widget::onFriendMessageReceived(uint32_t friendnumber, const QString& message, bool isAction)
{
    const auto& friendId = friendList->id2Key(friendnumber);
    Friend* f = friendList->findFriend(friendId);
    if (!f) {
        return;
    }

    friendMessageDispatchers[f->getPublicKey()]->onMessageReceived(isAction, message);
}

void Widget::onReceiptReceived(int friendId, ReceiptNum receipt)
{
    const auto& friendKey = friendList->id2Key(friendId);
    Friend* f = friendList->findFriend(friendKey);
    if (!f) {
        return;
    }

    friendMessageDispatchers[f->getPublicKey()]->onReceiptReceived(receipt);
}

void Widget::onExtendedMessageSupport(uint32_t friendNumber, bool supported)
{
    const auto& friendKey = friendList->id2Key(friendNumber);
    Friend* f = friendList->findFriend(friendKey);
    if (!f) {
        return;
    }

    f->setExtendedMessageSupport(supported);
}

void Widget::onFriendExtMessageReceived(uint32_t friendNumber, const QString& message)
{
    const auto& friendKey = friendList->id2Key(friendNumber);
    friendMessageDispatchers[friendKey]->onExtMessageReceived(message);
}

void Widget::onExtReceiptReceived(uint32_t friendNumber, uint64_t receiptId)
{
    const auto& friendKey = friendList->id2Key(friendNumber);
    friendMessageDispatchers[friendKey]->onExtReceiptReceived(receiptId);
}

void Widget::addFriendDialog(const Friend* frnd, ContentDialog* dialog)
{
    const ToxPk& friendPk = frnd->getPublicKey();
    ContentDialog* contentDialog = contentDialogManager->getFriendDialog(friendPk);
    bool isSeparate = settings.getSeparateWindow();
    FriendWidget* widget = friendWidgets[friendPk];
    bool isCurrent = activeChatroomWidget == widget;
    if (!contentDialog && !isSeparate && isCurrent) {
        onAddClicked();
    }

    auto form = chatForms[friendPk];
    auto chatroom = friendChatrooms[friendPk];
    FriendWidget* friendWidget =
        contentDialogManager->addFriendToDialog(dialog, chatroom, form);

    friendWidget->setStatusMsg(widget->getStatusMsg());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    auto widgetRemoveFriend = QOverload<const ToxPk&>::of(&Widget::removeFriend);
#else
    auto widgetRemoveFriend = static_cast<void (Widget::*)(const ToxPk&)>(&Widget::removeFriend);
#endif
    connect(friendWidget, &FriendWidget::removeFriend, this, widgetRemoveFriend);
    connect(friendWidget, &FriendWidget::middleMouseClicked, dialog,
            [=]() { dialog->removeFriend(friendPk); });
    connect(friendWidget, &FriendWidget::copyFriendIdToClipboard, this,
            &Widget::copyFriendIdToClipboard);
    connect(friendWidget, &FriendWidget::newWindowOpened, this, &Widget::openNewDialog);

    // Signal transmission from the created `friendWidget` (which shown in
    // ContentDialog) to the `widget` (which shown in main widget)
    // FIXME: emit should be removed
    connect(friendWidget, &FriendWidget::contextMenuCalled, widget,
            [=](QContextMenuEvent* event) { emit widget->contextMenuCalled(event); });

    connect(friendWidget, &FriendWidget::chatroomWidgetClicked, [=](GenericChatroomWidget* w) {
        std::ignore = w;
        emit widget->chatroomWidgetClicked(widget);
    });
    connect(friendWidget, &FriendWidget::newWindowOpened, [=](GenericChatroomWidget* w) {
        std::ignore = w;
        emit widget->newWindowOpened(widget);
    });
    // FIXME: emit should be removed
    emit widget->chatroomWidgetClicked(widget);

    connect(&profile, &Profile::friendAvatarSet, friendWidget, &FriendWidget::onAvatarSet);
    connect(&profile, &Profile::friendAvatarRemoved, friendWidget, &FriendWidget::onAvatarRemoved);

    QPixmap avatar = profile.loadAvatar(frnd->getPublicKey());
    if (!avatar.isNull()) {
        friendWidget->onAvatarSet(frnd->getPublicKey(), avatar);
    }
}

void Widget::addGroupDialog(const Group* group, ContentDialog* dialog)
{
    const GroupId& groupId = group->getPersistentId();
    ContentDialog* groupDialog = contentDialogManager->getGroupDialog(groupId);
    bool separated = settings.getSeparateWindow();
    GroupWidget* widget = groupWidgets[groupId];
    bool isCurrentWindow = activeChatroomWidget == widget;
    if (!groupDialog && !separated && isCurrentWindow) {
        onAddClicked();
    }

    auto chatForm = groupChatForms[groupId].data();
    auto chatroom = groupChatrooms[groupId];
    auto groupWidget =
        contentDialogManager->addGroupToDialog(dialog, chatroom, chatForm);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    auto removeGroup = QOverload<const GroupId&>::of(&Widget::removeGroup);
#else
    auto removeGroup = static_cast<void (Widget::*)(const GroupId&)>(&Widget::removeGroup);
#endif
    connect(groupWidget, &GroupWidget::removeGroup, this, removeGroup);
    connect(groupWidget, &GroupWidget::chatroomWidgetClicked, chatForm, &GroupChatForm::focusInput);
    connect(groupWidget, &GroupWidget::middleMouseClicked, dialog,
            [=]() { dialog->removeGroup(groupId); });
    connect(groupWidget, &GroupWidget::chatroomWidgetClicked, chatForm, &ChatForm::focusInput);
    connect(groupWidget, &GroupWidget::newWindowOpened, this, &Widget::openNewDialog);

    // Signal transmission from the created `groupWidget` (which shown in
    // ContentDialog) to the `widget` (which shown in main widget)
    // FIXME: emit should be removed
    connect(groupWidget, &GroupWidget::chatroomWidgetClicked, [=](GenericChatroomWidget* w) {
        std::ignore = w;
        emit widget->chatroomWidgetClicked(widget);
    });

    connect(groupWidget, &GroupWidget::newWindowOpened, [=](GenericChatroomWidget* w) {
        std::ignore = w;
        emit widget->newWindowOpened(widget);
    });

    // FIXME: emit should be removed
    emit widget->chatroomWidgetClicked(widget);
}

bool Widget::newFriendMessageAlert(const ToxPk& friendId, const QString& text, bool sound, QString filename, size_t filesize)
{
    bool hasActive;
    QWidget* currentWindow;
    ContentDialog* contentDialog = contentDialogManager->getFriendDialog(friendId);
    Friend* f = friendList->findFriend(friendId);

    if (contentDialog != nullptr) {
        currentWindow = contentDialog->window();
        hasActive = contentDialogManager->isChatActive(friendId);
    } else {
        if (settings.getSeparateWindow() && settings.getShowWindow()) {
            if (settings.getDontGroupWindows()) {
                contentDialog = createContentDialog();
            } else {
                contentDialog = contentDialogManager->current();
                if (!contentDialog) {
                    contentDialog = createContentDialog();
                }
            }

            addFriendDialog(f, contentDialog);
            currentWindow = contentDialog->window();
            hasActive = contentDialogManager->isChatActive(friendId);
        } else {
            currentWindow = window();
            FriendWidget* widget = friendWidgets[friendId];
            hasActive = widget == activeChatroomWidget;
        }
    }

    if (newMessageAlert(currentWindow, hasActive, sound)) {
        FriendWidget* widget = friendWidgets[friendId];
        f->setEventFlag(true);
        widget->updateStatusLight();
        ui->friendList->trackWidget(settings, style, widget);
#if DESKTOP_NOTIFICATIONS
        auto notificationData = filename.isEmpty() ? notificationGenerator->friendMessageNotification(f, text)
                                                   : notificationGenerator->fileTransferNotification(f, filename, filesize);
        notifier.notifyMessage(notificationData);
#else
        std::ignore = text;
        std::ignore = filename;
        std::ignore = filesize;
#endif

        if (contentDialog == nullptr) {
            if (hasActive) {
                setWindowTitle(widget->getTitle());
            }
        } else {
            contentDialogManager->updateFriendStatus(friendId);
        }

        return true;
    }

    return false;
}

bool Widget::newGroupMessageAlert(const GroupId& groupId, const ToxPk& authorPk,
                                  const QString& message, bool notify)
{
    bool hasActive;
    QWidget* currentWindow;
    ContentDialog* contentDialog = contentDialogManager->getGroupDialog(groupId);
    Group* g = groupList->findGroup(groupId);
    GroupWidget* widget = groupWidgets[groupId];

    if (contentDialog != nullptr) {
        currentWindow = contentDialog->window();
        hasActive = contentDialogManager->isChatActive(groupId);
    } else {
        currentWindow = window();
        hasActive = widget == activeChatroomWidget;
    }

    if (!newMessageAlert(currentWindow, hasActive, true, notify)) {
        return false;
    }

    g->setEventFlag(true);
    widget->updateStatusLight();
#if DESKTOP_NOTIFICATIONS
    auto notificationData = notificationGenerator->groupMessageNotification(g, authorPk, message);
    notifier.notifyMessage(notificationData);
#else
    std::ignore = authorPk;
    std::ignore = message;
#endif

    if (contentDialog == nullptr) {
        if (hasActive) {
            setWindowTitle(widget->getTitle());
        }
    } else {
        contentDialogManager->updateGroupStatus(groupId);
    }

    return true;
}

QString Widget::fromDialogType(DialogType type)
{
    switch (type) {
    case DialogType::AddDialog:
        return tr("Add friend", "title of the window");
    case DialogType::GroupDialog:
        return tr("Group invites", "title of the window");
    case DialogType::TransferDialog:
        return tr("File transfers", "title of the window");
    case DialogType::SettingDialog:
        return tr("Settings", "title of the window");
    case DialogType::ProfileDialog:
        return tr("My profile", "title of the window");
    }

    assert(false);
    return QString();
}

bool Widget::newMessageAlert(QWidget* currentWindow, bool isActive, bool sound, bool notify)
{
    bool inactiveWindow = isMinimized() || !currentWindow->isActiveWindow();

    if (!inactiveWindow && isActive) {
        return false;
    }

    if (notify) {
        if (settings.getShowWindow()) {
            currentWindow->show();
        }

        if (settings.getNotify()) {
            if (inactiveWindow) {
#if DESKTOP_NOTIFICATIONS
                if (!settings.getDesktopNotify()) {
                    QApplication::alert(currentWindow);
                }
#else
                QApplication::alert(currentWindow);
#endif
                eventFlag = true;
            }
            bool isBusy = core->getStatus() == Status::Status::Busy;
            bool busySound = settings.getBusySound();
            bool notifySound = settings.getNotifySound();

            if (notifySound && sound && (!isBusy || busySound)) {
                playNotificationSound(IAudioSink::Sound::NewMessage);
            }
        }
    }

    return true;
}

void Widget::onFriendRequestReceived(const ToxPk& friendPk, const QString& message)
{
    if (addFriendForm->addFriendRequest(friendPk.toString(), message)) {
        friendRequestsUpdate();
        newMessageAlert(window(), isActiveWindow(), true, true);
#if DESKTOP_NOTIFICATIONS
        auto notificationData = notificationGenerator->friendRequestNotification(friendPk, message);
        notifier.notifyMessage(notificationData);
#endif
    }
}

void Widget::onFileReceiveRequested(const ToxFile& file)
{
    const ToxPk& friendPk = friendList->id2Key(file.friendId);
    newFriendMessageAlert(friendPk, {}, true, file.fileName, file.progress.getFileSize());
}

void Widget::updateFriendActivity(const Friend& frnd)
{
    const ToxPk& pk = frnd.getPublicKey();
    const auto oldTime = settings.getFriendActivity(pk);
    const auto newTime = QDateTime::currentDateTime();
    settings.setFriendActivity(pk, newTime);
    FriendWidget* widget = friendWidgets[frnd.getPublicKey()];
    chatListWidget->moveWidget(widget, frnd.getStatus());
    chatListWidget->updateActivityTime(oldTime); // update old category widget
}

void Widget::removeFriend(Friend* f, bool fake)
{
    assert(f);
    if (!fake) {
        RemoveChatDialog ask(this, *f);
        ask.exec();

        if (!ask.accepted()) {
            return;
        }

        if (ask.removeHistory()) {
            profile.getHistory()->removeChatHistory(f->getPersistentId());
        }
    }

    const ToxPk friendPk = f->getPublicKey();
    auto widget = friendWidgets[friendPk];
    widget->setAsInactiveChatroom();
    if (widget == activeChatroomWidget) {
        activeChatroomWidget = nullptr;
        onAddClicked();
    }

    friendAlertConnections.remove(friendPk);

    chatListWidget->removeFriendWidget(widget);

    ContentDialog* lastDialog = contentDialogManager->getFriendDialog(friendPk);
    if (lastDialog != nullptr) {
        lastDialog->removeFriend(friendPk);
    }

    friendList->removeFriend(friendPk, settings, fake);
    if (!fake) {
        core->removeFriend(f->getId());
        // aliases aren't supported for non-friend peers in groups, revert to basic username
        for (Group* g : groupList->getAllGroups()) {
            if (g->getPeerList().contains(friendPk)) {
                g->updateUsername(friendPk, f->getUserName());
            }
        }
    }

    friendWidgets.remove(friendPk);

    auto chatForm = chatForms[friendPk];
    chatForms.remove(friendPk);
    delete chatForm;

    delete f;
    if (contentLayout && contentLayout->mainHead->layout()->isEmpty()) {
        onAddClicked();
    }
}

void Widget::removeFriend(const ToxPk& friendId)
{
    removeFriend(friendList->findFriend(friendId), false);
}

void Widget::onDialogShown(GenericChatroomWidget* widget)
{
    widget->resetEventFlags();
    widget->updateStatusLight();

    ui->friendList->updateTracking(widget);
    resetIcon();
}

void Widget::onFriendDialogShown(const Friend* f)
{
    onDialogShown(friendWidgets[f->getPublicKey()]);
}

void Widget::onGroupDialogShown(Group* g)
{
    const GroupId& groupId = g->getPersistentId();
    onDialogShown(groupWidgets[groupId]);
}

void Widget::toggleFullscreen()
{
    if (windowState().testFlag(Qt::WindowFullScreen)) {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() | Qt::WindowFullScreen);
    }
}

void Widget::onUpdateAvailable()
{
    ui->settingsButton->setProperty("update-available", true);
    ui->settingsButton->style()->unpolish(ui->settingsButton);
    ui->settingsButton->style()->polish(ui->settingsButton);
}

ContentDialog* Widget::createContentDialog() const
{
    ContentDialog* contentDialog = new ContentDialog(*core, settings, style,
        *messageBoxManager, *friendList, *groupList, profile);

    registerContentDialog(*contentDialog);
    return contentDialog;
}

void Widget::registerContentDialog(ContentDialog& contentDialog) const
{
    contentDialogManager->addContentDialog(contentDialog);
    connect(&contentDialog, &ContentDialog::friendDialogShown, this, &Widget::onFriendDialogShown);
    connect(&contentDialog, &ContentDialog::groupDialogShown, this, &Widget::onGroupDialogShown);
    connect(core, &Core::usernameSet, &contentDialog, &ContentDialog::setUsername);
    connect(&settings, &Settings::groupchatPositionChanged, &contentDialog,
            &ContentDialog::reorderLayouts);
    connect(&contentDialog, &ContentDialog::addFriendDialog, this, &Widget::addFriendDialog);
    connect(&contentDialog, &ContentDialog::addGroupDialog, this, &Widget::addGroupDialog);
    connect(&contentDialog, &ContentDialog::connectFriendWidget, this, &Widget::connectFriendWidget);

#ifdef Q_OS_MAC
    Nexus& n = nexus;
    connect(&contentDialog, &ContentDialog::destroyed, &n, &Nexus::updateWindowsClosed);
    connect(&contentDialog, &ContentDialog::windowStateChanged, &n, &Nexus::onWindowStateChanged);
    connect(contentDialog.windowHandle(), &QWindow::windowTitleChanged, &n, &Nexus::updateWindows);
    n.updateWindows();
#endif
}

ContentLayout* Widget::createContentDialog(DialogType type) const
{
    class Dialog : public ActivateDialog
    {
    public:
        explicit Dialog(DialogType type_, Settings& settings_, Core* core_, Style& style_)
            : ActivateDialog(style_, nullptr, Qt::Window)
            , type(type_)
            , settings(settings_)
            , core{core_}
            , style{style_}
        {
            restoreGeometry(settings.getDialogSettingsGeometry());
            Translator::registerHandler(std::bind(&Dialog::retranslateUi, this), this);
            retranslateUi();
            setWindowIcon(QIcon(":/img/icons/qtox.svg"));
            reloadTheme();

            connect(core, &Core::usernameSet, this, &Dialog::retranslateUi);
        }

        ~Dialog()
        {
            Translator::unregister(this);
        }

    public slots:

        void retranslateUi()
        {
            setWindowTitle(core->getUsername() + QStringLiteral(" - ") + Widget::fromDialogType(type));
        }

        void reloadTheme() final
        {
            setStyleSheet(style.getStylesheet("window/general.css", settings));
        }

    protected:
        void resizeEvent(QResizeEvent* event) override
        {
            settings.setDialogSettingsGeometry(saveGeometry());
            QDialog::resizeEvent(event);
        }

        void moveEvent(QMoveEvent* event) override
        {
            settings.setDialogSettingsGeometry(saveGeometry());
            QDialog::moveEvent(event);
        }

    private:
        DialogType type;
        Settings& settings;
        Core* core;
        Style& style;
    };

    Dialog* dialog = new Dialog(type, settings, core, style);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    ContentLayout* contentLayoutDialog = new ContentLayout(settings, style, dialog);

    dialog->setObjectName("detached");
    dialog->setLayout(contentLayoutDialog);
    dialog->layout()->setMargin(0);
    dialog->layout()->setSpacing(0);
    dialog->setMinimumSize(720, 400);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();

#ifdef Q_OS_MAC
    connect(dialog, &Dialog::destroyed, &nexus, &Nexus::updateWindowsClosed);
    connect(dialog, &ActivateDialog::windowStateChanged, &nexus,
            &Nexus::updateWindowsStates);
    connect(dialog->windowHandle(), &QWindow::windowTitleChanged, &nexus,
            &Nexus::updateWindows);
    nexus.updateWindows();
#endif

    return contentLayoutDialog;
}

void Widget::copyFriendIdToClipboard(const ToxPk& friendId)
{
    Friend* f = friendList->findFriend(friendId);
    if (f != nullptr) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(friendId.toString(), QClipboard::Clipboard);
    }
}

void Widget::onGroupInviteReceived(const GroupInvite& inviteInfo)
{
    const uint32_t friendId = inviteInfo.getFriendId();
    const ToxPk& friendPk = friendList->id2Key(friendId);
    const Friend* f = friendList->findFriend(friendPk);
    updateFriendActivity(*f);

    const uint8_t confType = inviteInfo.getType();
    if (confType == TOX_CONFERENCE_TYPE_TEXT || confType == TOX_CONFERENCE_TYPE_AV) {
        if (settings.getAutoGroupInvite(f->getPublicKey())) {
            onGroupInviteAccepted(inviteInfo);
        } else {
            if (!groupInviteForm->addGroupInvite(inviteInfo)) {
                return;
            }

            ++unreadGroupInvites;
            groupInvitesUpdate();
            newMessageAlert(window(), isActiveWindow(), true, true);
#if DESKTOP_NOTIFICATIONS
            auto notificationData = notificationGenerator->groupInvitationNotification(f);
            notifier.notifyMessage(notificationData);
#endif
        }
    } else {
        qWarning() << "onGroupInviteReceived: Unknown groupchat type:" << confType;
        return;
    }
}

void Widget::onGroupInviteAccepted(const GroupInvite& inviteInfo)
{
    const uint32_t groupId = core->joinGroupchat(inviteInfo);
    if (groupId == std::numeric_limits<uint32_t>::max()) {
        qWarning() << "onGroupInviteAccepted: Unable to accept group invite";
        return;
    }
}

void Widget::onGroupMessageReceived(int groupnumber, int peernumber, const QString& message,
                                    bool isAction)
{
    const GroupId& groupId = groupList->id2Key(groupnumber);
    assert(groupList->findGroup(groupId));

    ToxPk author = core->getGroupPeerPk(groupnumber, peernumber);

    groupMessageDispatchers[groupId]->onMessageReceived(author, isAction, message);
}

void Widget::onGroupPeerlistChanged(uint32_t groupnumber)
{
    const GroupId& groupId = groupList->id2Key(groupnumber);
    Group* g = groupList->findGroup(groupId);
    assert(g);
    g->regeneratePeerList();
}

void Widget::onGroupPeerNameChanged(uint32_t groupnumber, const ToxPk& peerPk, const QString& newName)
{
    const GroupId& groupId = groupList->id2Key(groupnumber);
    Group* g = groupList->findGroup(groupId);
    assert(g);

    const QString setName = friendList->decideNickname(peerPk, newName);
    g->updateUsername(peerPk, newName);
}

void Widget::onGroupTitleChanged(uint32_t groupnumber, const QString& author, const QString& title)
{
    const GroupId& groupId = groupList->id2Key(groupnumber);
    Group* g = groupList->findGroup(groupId);
    assert(g);

    GroupWidget* widget = groupWidgets[groupId];
    if (widget->isActive()) {
        formatWindowTitle(title);
    }

    g->setTitle(author, title);
    chatListWidget->itemsChanged();
}

void Widget::titleChangedByUser(const QString& title)
{
    const auto* group = qobject_cast<Group*>(sender());
    assert(group != nullptr);
    emit changeGroupTitle(group->getId(), title);
}

void Widget::onGroupPeerAudioPlaying(int groupnumber, ToxPk peerPk)
{
    const GroupId& groupId = groupList->id2Key(groupnumber);
    assert(groupList->findGroup(groupId));

    auto form = groupChatForms[groupId].data();
    form->peerAudioPlaying(peerPk);
}

void Widget::removeGroup(Group* g, bool fake)
{
    assert(g);
    if (!fake) {
        RemoveChatDialog ask(this, *g);
        ask.exec();

        if (!ask.accepted()) {
            return;
        }

        if (ask.removeHistory()) {
            profile.getHistory()->removeChatHistory(g->getPersistentId());
        }
    }

    const auto& groupId = g->getPersistentId();
    const auto groupnumber = g->getId();
    auto groupWidgetIt = groupWidgets.find(groupId);
    if (groupWidgetIt == groupWidgets.end()) {
        qWarning() << "Tried to remove group" << groupnumber << "but GroupWidget doesn't exist";
        return;
    }
    auto widget = groupWidgetIt.value();
    widget->setAsInactiveChatroom();
    if (static_cast<GenericChatroomWidget*>(widget) == activeChatroomWidget) {
        activeChatroomWidget = nullptr;
        onAddClicked();
    }

    groupList->removeGroup(groupId, fake);
    ContentDialog* contentDialog = contentDialogManager->getGroupDialog(groupId);
    if (contentDialog != nullptr) {
        contentDialog->removeGroup(groupId);
    }

    if (!fake) {
        core->removeGroup(groupnumber);
    }
    chatListWidget->removeGroupWidget(widget); // deletes widget

    groupWidgets.remove(groupId);
    auto groupChatFormIt = groupChatForms.find(groupId);
    if (groupChatFormIt == groupChatForms.end()) {
        qWarning() << "Tried to remove group" << groupnumber << "but GroupChatForm doesn't exist";
        return;
    }
    groupChatForms.erase(groupChatFormIt);
    groupAlertConnections.remove(groupId);

    delete g;
    if (contentLayout && contentLayout->mainHead->layout()->isEmpty()) {
        onAddClicked();
    }

}

void Widget::removeGroup(const GroupId& groupId)
{
    removeGroup(groupList->findGroup(groupId));
}

Group* Widget::createGroup(uint32_t groupnumber, const GroupId& groupId)
{
    assert(core != nullptr);

    Group* g = groupList->findGroup(groupId);
    if (g) {
        qWarning() << "Group already exists";
        return g;
    }

    const auto groupName = tr("Groupchat #%1").arg(groupnumber);
    const bool enabled = core->getGroupAvEnabled(groupnumber);
    Group* newgroup =
        groupList->addGroup(*core, groupnumber, groupId, groupName, enabled, core->getUsername(),
            *friendList);
    assert(newgroup);

    if (enabled) {
        connect(newgroup, &Group::userLeft, [=](const ToxPk& user){
            CoreAV *av = core->getAv();
            assert(av);
            av->invalidateGroupCallPeerSource(*newgroup, user);
        });
    }
    auto rawChatroom = new GroupChatroom(newgroup, contentDialogManager.get(), *core,
        *friendList);
    std::shared_ptr<GroupChatroom> chatroom(rawChatroom);

    const auto compact = settings.getCompactLayout();
    auto widget = new GroupWidget(chatroom, compact, settings, style);
    auto messageProcessor = MessageProcessor(*sharedMessageProcessorParams);
    auto messageDispatcher =
        std::make_shared<GroupMessageDispatcher>(*newgroup, std::move(messageProcessor), *core,
                                                 *core, settings);

    auto history = profile.getHistory();
    // Note: We do not have to connect the message dispatcher signals since
    // ChatHistory hooks them up in a very specific order
    auto chatHistory =
        std::make_shared<ChatHistory>(*newgroup, history, *core, settings,
                                      *messageDispatcher, *friendList, *groupList);

    auto notifyReceivedCallback = [this, groupId](const ToxPk& author, const Message& message) {
        auto isTargeted = std::any_of(message.metadata.begin(), message.metadata.end(),
                                      [](MessageMetadata metadata) {
                                          return metadata.type == MessageMetadataType::selfMention;
                                      });
        newGroupMessageAlert(groupId, author, message.content,
                             isTargeted || settings.getGroupAlwaysNotify());
    };

    auto notifyReceivedConnection =
        connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived, notifyReceivedCallback);
    groupAlertConnections.insert(groupId, notifyReceivedConnection);

    auto form = new GroupChatForm(*core, newgroup, *chatHistory, *messageDispatcher,
        settings, *documentCache, *smileyPack, style, *messageBoxManager, *friendList, *groupList);
    connect(&settings, &Settings::nameColorsChanged, form, &GenericChatForm::setColorizedNames);
    form->setColorizedNames(settings.getEnableGroupChatsColor());
    groupMessageDispatchers[groupId] = messageDispatcher;
    groupChatLogs[groupId] = chatHistory;
    groupWidgets[groupId] = widget;
    groupChatrooms[groupId] = chatroom;
    groupChatForms[groupId] = QSharedPointer<GroupChatForm>(form);

    chatListWidget->addGroupWidget(widget);
    widget->updateStatusLight();
    chatListWidget->activateWindow();

    connect(widget, &GroupWidget::chatroomWidgetClicked, this, &Widget::onChatroomWidgetClicked);
    connect(widget, &GroupWidget::newWindowOpened, this, &Widget::openNewDialog);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    auto widgetRemoveGroup = QOverload<const GroupId&>::of(&Widget::removeGroup);
#else
    auto widgetRemoveGroup = static_cast<void (Widget::*)(const GroupId&)>(&Widget::removeGroup);
#endif
    connect(widget, &GroupWidget::removeGroup, this, widgetRemoveGroup);
    connect(widget, &GroupWidget::middleMouseClicked, this, [=]() { removeGroup(groupId); });
    connect(widget, &GroupWidget::chatroomWidgetClicked, form, &ChatForm::focusInput);
    connect(newgroup, &Group::titleChangedByUser, this, &Widget::titleChangedByUser);
    connect(core, &Core::usernameSet, newgroup, &Group::setSelfName);

    return newgroup;
}

void Widget::onEmptyGroupCreated(uint32_t groupnumber, const GroupId& groupId, const QString& title)
{
    Group* group = createGroup(groupnumber, groupId);
    if (!group) {
        return;
    }
    if (title.isEmpty()) {
        // Only rename group if groups are visible.
        if (groupsVisible()) {
            groupWidgets[groupId]->editName();
        }
    } else {
        group->setTitle(QString(), title);
    }
}

void Widget::onGroupJoined(int groupNum, const GroupId& groupId)
{
    createGroup(groupNum, groupId);
}

/**
 * @brief Used to reset the blinking icon.
 */
void Widget::resetIcon()
{
    eventIcon = false;
    eventFlag = false;
    updateIcons();
}

bool Widget::event(QEvent* e)
{
    switch (e->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
        focusChatInput();
        break;
    case QEvent::Paint:
        ui->friendList->updateVisualTracking();
        break;
    case QEvent::WindowActivate:
        if (activeChatroomWidget) {
            activeChatroomWidget->resetEventFlags();
            activeChatroomWidget->updateStatusLight();
            setWindowTitle(activeChatroomWidget->getTitle());
        }

        if (eventFlag) {
            resetIcon();
        }

        focusChatInput();

#ifdef Q_OS_MAC
        emit windowStateChanged(windowState());

    case QEvent::WindowStateChange:
        nexus.updateWindowsStates();
#endif
        break;
    default:
        break;
    }

    return QMainWindow::event(e);
}

void Widget::onUserAwayCheck()
{
#ifdef QTOX_PLATFORM_EXT
    uint32_t autoAwayTime = settings.getAutoAwayTime() * 60 * 1000;
    bool online = static_cast<Status::Status>(ui->statusButton->property("status").toInt())
                  == Status::Status::Online;
    bool away = autoAwayTime && Platform::getIdleTime() >= autoAwayTime;

    if (online && away) {
        qDebug() << "auto away activated at" << QTime::currentTime().toString();
        emit statusSet(Status::Status::Away);
        autoAwayActive = true;
    } else if (autoAwayActive && !away) {
        qDebug() << "auto away deactivated at" << QTime::currentTime().toString();
        emit statusSet(Status::Status::Online);
        autoAwayActive = false;
    }
#endif
}

void Widget::onEventIconTick()
{
    if (eventFlag) {
        eventIcon ^= true;
        updateIcons();
    }
}

void Widget::onTryCreateTrayIcon()
{
    static int32_t tries = 15;
    if (!icon && tries--) {
        if (QSystemTrayIcon::isSystemTrayAvailable()) {
            icon = std::unique_ptr<QSystemTrayIcon>(new QSystemTrayIcon);
            updateIcons();
            trayMenu = new QMenu(this);

            // adding activate to the top, avoids accidentally clicking quit
            trayMenu->addAction(actionShow);
            trayMenu->addSeparator();
            trayMenu->addAction(statusOnline);
            trayMenu->addAction(statusAway);
            trayMenu->addAction(statusBusy);
            trayMenu->addSeparator();
            trayMenu->addAction(actionLogout);
            trayMenu->addAction(actionQuit);
            icon->setContextMenu(trayMenu);

            connect(icon.get(), &QSystemTrayIcon::activated, this, &Widget::onIconClick);

            if (settings.getShowSystemTray()) {
                icon->show();
                setHidden(settings.getAutostartInTray());
            } else {
                show();
            }

#ifdef Q_OS_MAC
            nexus.dockMenu->setAsDockMenu();
#endif
        } else if (!isVisible()) {
            show();
        }
    } else {
        disconnect(timer, &QTimer::timeout, this, &Widget::onTryCreateTrayIcon);
        if (!icon) {
            qWarning() << "No system tray detected!";
            show();
        }
    }
}

void Widget::setStatusOnline()
{
    if (!ui->statusButton->isEnabled()) {
        return;
    }

    core->setStatus(Status::Status::Online);
}

void Widget::setStatusAway()
{
    if (!ui->statusButton->isEnabled()) {
        return;
    }

    core->setStatus(Status::Status::Away);
}

void Widget::setStatusBusy()
{
    if (!ui->statusButton->isEnabled()) {
        return;
    }

    core->setStatus(Status::Status::Busy);
}

void Widget::onGroupSendFailed(uint32_t groupnumber)
{
    const auto& groupId = groupList->id2Key(groupnumber);
    assert(groupList->findGroup(groupId));

    const auto curTime = QDateTime::currentDateTime();
    auto form = groupChatForms[groupId].data();
    form->addSystemInfoMessage(curTime, SystemMessageType::messageSendFailed, {});
}

void Widget::onFriendTypingChanged(uint32_t friendnumber, bool isTyping)
{
    const auto& friendId = friendList->id2Key(friendnumber);
    Friend* f = friendList->findFriend(friendId);
    if (!f) {
        return;
    }

    chatForms[f->getPublicKey()]->setFriendTyping(isTyping);
}

void Widget::onSetShowSystemTray(bool newValue)
{
    if (icon) {
        icon->setVisible(newValue);
    }
}

void Widget::saveWindowGeometry()
{
    settings.setWindowGeometry(saveGeometry());
    settings.setWindowState(saveState());
}

void Widget::saveSplitterGeometry()
{
    if (!settings.getSeparateWindow()) {
        settings.setSplitterState(ui->mainSplitter->saveState());
    }
}

void Widget::onSplitterMoved(int pos, int index)
{
    std::ignore = pos;
    std::ignore = index;
    saveSplitterGeometry();
}

void Widget::cycleChats(bool forward)
{
    chatListWidget->cycleChats(activeChatroomWidget, forward);
}

bool Widget::filterGroups(FilterCriteria index)
{
    switch (index) {
    case FilterCriteria::Offline:
    case FilterCriteria::Friends:
        return true;
    default:
        return false;
    }
}

bool Widget::filterOffline(FilterCriteria index)
{
    switch (index) {
    case FilterCriteria::Online:
    case FilterCriteria::Groups:
        return true;
    default:
        return false;
    }
}

bool Widget::filterOnline(FilterCriteria index)
{
    switch (index) {
    case FilterCriteria::Offline:
    case FilterCriteria::Groups:
        return true;
    default:
        return false;
    }
}

void Widget::clearAllReceipts()
{
    QList<Friend*> frnds = friendList->getAllFriends();
    for (Friend* f : frnds) {
        friendMessageDispatchers[f->getPublicKey()]->clearOutgoingMessages();
    }
}

void Widget::reloadTheme()
{
    setStyleSheet("");
    QWidgetList wgts = findChildren<QWidget*>();
    for (auto x : wgts) {
        x->setStyleSheet("");
    }

    setStyleSheet(style.getStylesheet("window/general.css", settings));
    QString statusPanelStyle = style.getStylesheet("window/statusPanel.css", settings);
    ui->tooliconsZone->setStyleSheet(style.getStylesheet("tooliconsZone/tooliconsZone.css", settings));
    ui->statusPanel->setStyleSheet(statusPanelStyle);
    ui->statusHead->setStyleSheet(statusPanelStyle);
    ui->friendList->setStyleSheet(style.getStylesheet("friendList/friendList.css", settings));
    ui->statusButton->setStyleSheet(style.getStylesheet("statusButton/statusButton.css", settings));

    profilePicture->setStyleSheet(style.getStylesheet("window/profile.css", settings));
}

void Widget::nextChat()
{
    cycleChats(true);
}

void Widget::previousChat()
{
    cycleChats(false);
}

// Preparing needed to set correct size of icons for GTK tray backend
inline QIcon Widget::prepareIcon(QString path, int w, int h)
{
#ifdef Q_OS_LINUX

    QString desktop = QString::fromUtf8(getenv("XDG_CURRENT_DESKTOP"));
    if (desktop.isEmpty()) {
        desktop = QString::fromUtf8(getenv("DESKTOP_SESSION"));
    }

    desktop = desktop.toLower();
    if (desktop == "xfce" || desktop.contains("gnome") || desktop == "mate" || desktop == "x-cinnamon") {
        if (w > 0 && h > 0) {
            QSvgRenderer renderer(path);

            QPixmap pm(w, h);
            pm.fill(Qt::transparent);
            QPainter painter(&pm);
            renderer.render(&painter, pm.rect());

            return QIcon(pm);
        }
    }
#else
    std::ignore = w;
    std::ignore = h;
#endif
    return QIcon(path);
}

void Widget::searchChats()
{
    QString searchString = ui->searchContactText->text();
    FilterCriteria filter = getFilterCriteria();

    chatListWidget->searchChatrooms(searchString, filterOnline(filter), filterOffline(filter),
                                       filterGroups(filter));

    updateFilterText();
}

void Widget::changeDisplayMode()
{
    filterDisplayGroup->setEnabled(false);

    if (filterDisplayGroup->checkedAction() == filterDisplayActivity) {
        chatListWidget->setMode(FriendListWidget::SortingMode::Activity);
    } else if (filterDisplayGroup->checkedAction() == filterDisplayName) {
        chatListWidget->setMode(FriendListWidget::SortingMode::Name);
    }

    searchChats();
    filterDisplayGroup->setEnabled(true);

    updateFilterText();
}

void Widget::updateFilterText()
{
    QString action = filterDisplayGroup->checkedAction()->text();
    QString text = filterGroup->checkedAction()->text();
    text = action + QStringLiteral(" | ") + text;
    ui->searchContactFilterBox->setText(text);
}

Widget::FilterCriteria Widget::getFilterCriteria() const
{
    QAction* checked = filterGroup->checkedAction();

    if (checked == filterOnlineAction)
        return FilterCriteria::Online;
    else if (checked == filterOfflineAction)
        return FilterCriteria::Offline;
    else if (checked == filterFriendsAction)
        return FilterCriteria::Friends;
    else if (checked == filterGroupsAction)
        return FilterCriteria::Groups;

    return FilterCriteria::All;
}

void Widget::searchCircle(CircleWidget& circleWidget)
{
    if (chatListWidget->getMode() == FriendListWidget::SortingMode::Name) {
        FilterCriteria filter = getFilterCriteria();
        QString text = ui->searchContactText->text();
        circleWidget.search(text, true, filterOnline(filter), filterOffline(filter));
    }
}

bool Widget::groupsVisible() const
{
    FilterCriteria filter = getFilterCriteria();
    return !filterGroups(filter);
}

void Widget::friendListContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QAction* createGroupAction = menu.addAction(tr("Create new group..."));
    QAction* addCircleAction = menu.addAction(tr("Add new circle..."));
    QAction* chosenAction = menu.exec(ui->friendList->mapToGlobal(pos));

    if (chosenAction == addCircleAction) {
        chatListWidget->addCircleWidget();
    } else if (chosenAction == createGroupAction) {
        core->createGroup();
    }
}

void Widget::friendRequestsUpdate()
{
    unsigned int unreadFriendRequests = settings.getUnreadFriendRequests();

    if (unreadFriendRequests == 0) {
        delete friendRequestsButton;
        friendRequestsButton = nullptr;
    } else if (!friendRequestsButton) {
        friendRequestsButton = new QPushButton(this);
        friendRequestsButton->setObjectName("green");
        ui->statusLayout->insertWidget(2, friendRequestsButton);

        connect(friendRequestsButton, &QPushButton::released, [this]() {
            onAddClicked();
            addFriendForm->setMode(AddFriendForm::Mode::FriendRequest);
        });
    }

    if (friendRequestsButton) {
        friendRequestsButton->setText(tr("%n new friend request(s)", "", unreadFriendRequests));
    }
}

void Widget::groupInvitesUpdate()
{
    if (unreadGroupInvites == 0) {
        delete groupInvitesButton;
        groupInvitesButton = nullptr;
    } else if (!groupInvitesButton) {
        groupInvitesButton = new QPushButton(this);
        groupInvitesButton->setObjectName("green");
        ui->statusLayout->insertWidget(2, groupInvitesButton);

        connect(groupInvitesButton, &QPushButton::released, this, &Widget::onGroupClicked);
    }

    if (groupInvitesButton) {
        groupInvitesButton->setText(tr("%n new group invite(s)", "", unreadGroupInvites));
    }
}

void Widget::groupInvitesClear()
{
    unreadGroupInvites = 0;
    groupInvitesUpdate();
}

void Widget::setActiveToolMenuButton(ActiveToolMenuButton newActiveButton)
{
    ui->addButton->setChecked(newActiveButton == ActiveToolMenuButton::AddButton);
    ui->addButton->setDisabled(newActiveButton == ActiveToolMenuButton::AddButton);
    ui->groupButton->setChecked(newActiveButton == ActiveToolMenuButton::GroupButton);
    ui->groupButton->setDisabled(newActiveButton == ActiveToolMenuButton::GroupButton);
    ui->transferButton->setChecked(newActiveButton == ActiveToolMenuButton::TransferButton);
    ui->transferButton->setDisabled(newActiveButton == ActiveToolMenuButton::TransferButton);
    ui->settingsButton->setChecked(newActiveButton == ActiveToolMenuButton::SettingButton);
    ui->settingsButton->setDisabled(newActiveButton == ActiveToolMenuButton::SettingButton);
}

void Widget::retranslateUi()
{
    ui->retranslateUi(this);
    setUsername(core->getUsername());
    setStatusMessage(core->getStatusMessage());

    filterDisplayName->setText(tr("By Name"));
    filterDisplayActivity->setText(tr("By Activity"));
    filterAllAction->setText(tr("All"));
    filterOnlineAction->setText(tr("Online"));
    filterOfflineAction->setText(tr("Offline"));
    filterFriendsAction->setText(tr("Friends"));
    filterGroupsAction->setText(tr("Groups"));
    ui->searchContactText->setPlaceholderText(tr("Search Contacts"));
    updateFilterText();

    statusOnline->setText(tr("Online", "Button to set your status to 'Online'"));
    statusAway->setText(tr("Away", "Button to set your status to 'Away'"));
    statusBusy->setText(tr("Busy", "Button to set your status to 'Busy'"));
    actionLogout->setText(tr("Logout", "Tray action menu to logout user"));
    actionQuit->setText(tr("Exit", "Tray action menu to exit Tox"));
    actionShow->setText(tr("Show", "Tray action menu to show qTox window"));

    if (!settings.getSeparateWindow() && (settingsWidget && settingsWidget->isShown())) {
        setWindowTitle(fromDialogType(DialogType::SettingDialog));
    }

    friendRequestsUpdate();
    groupInvitesUpdate();


#ifdef Q_OS_MAC
    nexus.retranslateUi();

    filterMenu->menuAction()->setText(tr("Filter..."));

    fileMenu->setText(tr("File"));
    editMenu->setText(tr("Edit"));
    contactMenu->setText(tr("Contacts"));
    changeStatusMenu->menuAction()->setText(tr("Change status"));
    editProfileAction->setText(tr("Edit profile"));
    logoutAction->setText(tr("Logout"));
    addContactAction->setText(tr("Add contact..."));
    nextConversationAction->setText(tr("Next conversation"));
    previousConversationAction->setText(tr("Previous conversation"));
#endif
}

void Widget::focusChatInput()
{
    if (activeChatroomWidget) {
        if (const Friend* f = activeChatroomWidget->getFriend()) {
            chatForms[f->getPublicKey()]->focusInput();
        } else if (Group* g = activeChatroomWidget->getGroup()) {
            groupChatForms[g->getPersistentId()]->focusInput();
        }
    }
}

void Widget::refreshPeerListsLocal(const QString& username)
{
    for (Group* g : groupList->getAllGroups()) {
        g->updateUsername(core->getSelfPublicKey(), username);
    }
}

void Widget::connectCircleWidget(CircleWidget& circleWidget)
{
    connect(&circleWidget, &CircleWidget::newContentDialog, this, &Widget::registerContentDialog);
}

void Widget::connectFriendWidget(FriendWidget& friendWidget)
{
    connect(&friendWidget, &FriendWidget::updateFriendActivity, this, &Widget::updateFriendActivity);
}

/**
 * @brief Change the title of the main window.
 * @param title Title to set.
 *
 * This is usually always visible to the user.
 */
void Widget::formatWindowTitle(const QString& content)
{
    if (content.isEmpty()) {
        setWindowTitle("qTox");
    } else {
        setWindowTitle(content + " - qTox");
    }
}

void Widget::registerIpcHandlers()
{
    ipc.registerEventHandler(activateHandlerKey, &toxActivateEventHandler, this);
    ipc.registerEventHandler(saveHandlerKey, &ToxSave::toxSaveEventHandler, toxSave.get());
}

bool Widget::handleToxSave(const QString& path)
{
    return toxSave->handleToxSave(path);
}
