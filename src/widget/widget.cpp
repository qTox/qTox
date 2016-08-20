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
#endif

#include "circlewidget.h"
#include "contentdialog.h"
#include "contentlayout.h"
#include "form/groupchatform.h"
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "groupwidget.h"
#include "maskablepixmapwidget.h"
#include "src/audio/audio.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/net/autoupdate.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/platform/timer.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/chatform.h"
#include "src/widget/form/filesform.h"
#include "src/widget/form/groupinviteform.h"
#include "src/widget/form/profileform.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "systemtrayicon.h"
#include "tool/removefrienddialog.h"

bool toxActivateEventHandler(const QByteArray&)
{
    Widget* widget = Nexus::getDesktopGUI();
    if (!widget)
        return true;
    if (!widget->isActiveWindow())
        widget->forceShow();

    return true;
}

Widget *Widget::instance{nullptr};

Widget::Widget(QWidget *parent)
    : QMainWindow(parent)
    , icon{nullptr}
    , trayMenu{nullptr}
    , contentArrangement(Qt::LeftEdge)
    , eventFlag(false)
    , eventIcon(false)
{
    installEventFilter(this);
    Translator::translate();
}

void Widget::init()
{
    setupUi(this);

    QIcon themeIcon = QIcon::fromTheme("qtox");
    if (!themeIcon.isNull())
        setWindowIcon(themeIcon);

    timer = new QTimer();
    timer->start(1000);
    offlineMsgTimer = new QTimer();
    // FIXME: ↓ make a proper fix instead of increasing timeout into ∞
    offlineMsgTimer->start(2*60*1000);

    icon_size = 15;

    actionShow = new QAction(this);
    connect(actionShow, &QAction::triggered, this, &Widget::forceShow);

    statusOnline = new QAction(this);
    statusOnline->setIcon(prepareIcon(getStatusIconPath(Status::Online), icon_size, icon_size));
    connect(statusOnline, &QAction::triggered, this, &Widget::setStatusOnline);

    statusAway = new QAction(this);
    statusAway->setIcon(prepareIcon(getStatusIconPath(Status::Away), icon_size, icon_size));
    connect(statusAway, &QAction::triggered, this, &Widget::setStatusAway);

    statusBusy = new QAction(this);
    statusBusy->setIcon(prepareIcon(getStatusIconPath(Status::Busy), icon_size, icon_size));
    connect(statusBusy, &QAction::triggered, this, &Widget::setStatusBusy);

    actionLogout = new QAction(this);
    actionLogout->setIcon(prepareIcon(":/img/others/logout-icon.svg", icon_size, icon_size));
    connect(actionLogout, &QAction::triggered, this, [&]() {
        // TODO: move logout to a global accessible place (e.g. Core).
        Settings::getInstance().saveGlobal();
        Nexus::getInstance().showLoginLater();
    });

    actionQuit = new QAction(this);
#ifndef Q_OS_OSX
    actionQuit->setMenuRole(QAction::QuitRole);
#endif

    actionQuit->setIcon(prepareIcon(":/ui/rejectCall/rejectCall.svg", icon_size, icon_size));
    connect(actionQuit, &QAction::triggered, qApp, &QApplication::quit);

    layout()->setContentsMargins(0, 0, 0, 0);
    friendList->setStyleSheet(Style::resolve(Style::getStylesheet(":/ui/friendList/friendList.css")));

    profilePicture = new MaskablePixmapWidget(this, QSize(40, 40), ":/img/avatar_mask.svg");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.svg"));
    profilePicture->setClickable(true);
    myProfile->insertWidget(0, profilePicture);
    myProfile->insertSpacing(1, 7);

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

    searchContactFilterBox->setMenu(filterMenu);

#ifndef Q_OS_MAC
    statusHead->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));
#endif

    contactListWidget = new FriendListWidget(this, Settings::getInstance().getGroupchatPosition());
    friendList->setWidget(contactListWidget);
    friendList->setLayoutDirection(Qt::RightToLeft);
    friendList->setContextMenuPolicy(Qt::CustomContextMenu);

    statusLabel->setEditable(true);
    statusPanel->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));

    addButton->setCheckable(true);
    groupButton->setCheckable(true);
    transferButton->setCheckable(true);
    settingsButton->setCheckable(true);

    QMenu *statusButtonMenu = new QMenu(statusButton);
    statusButtonMenu->addAction(statusOnline);
    statusButtonMenu->addAction(statusAway);
    statusButtonMenu->addAction(statusBusy);
    statusButton->setMenu(statusButtonMenu);

    // disable proportional scaling
    mainSplitter->setStretchFactor(0,0);
    mainSplitter->setStretchFactor(1,1);

    onStatusSet(Status::Offline);

    // Disable some widgets until we're connected to the DHT
    statusButton->setEnabled(false);

    Style::setThemeColor(Settings::getInstance().getThemeColor());
    reloadTheme();
    updateIcons();

    connect(addButton, &QPushButton::clicked, this, &Widget::onAddClicked);
    connect(groupButton, &QPushButton::clicked, this, &Widget::onGroupClicked);
    connect(transferButton, &QPushButton::clicked, this, &Widget::onTransferClicked);
    connect(settingsButton, &QPushButton::clicked, this, &Widget::onShowSettings);
    connect(profilePicture, &MaskablePixmapWidget::clicked, this, &Widget::showProfile);
    connect(nameLabel, &CroppingLabel::clicked, this, &Widget::showProfile);
    connect(statusLabel, &CroppingLabel::editFinished, this, &Widget::onStatusMessageChanged);
    connect(mainSplitter, &QSplitter::splitterMoved, this, &Widget::onSplitterMoved);
    connect(timer, &QTimer::timeout, this, &Widget::onUserAwayCheck);
    connect(timer, &QTimer::timeout, this, &Widget::onEventIconTick);
    connect(timer, &QTimer::timeout, this, &Widget::onTryCreateTrayIcon);
    connect(offlineMsgTimer, &QTimer::timeout, this, &Widget::processOfflineMsgs);
    connect(searchContactText, &QLineEdit::textChanged, this, &Widget::searchContacts);
    connect(filterGroup, &QActionGroup::triggered, this, &Widget::searchContacts);
    connect(filterDisplayGroup, &QActionGroup::triggered, this, &Widget::changeDisplayMode);
    connect(friendList, &QWidget::customContextMenuRequested, this, &Widget::friendListContextMenu);

    // keyboard shortcuts
    new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(close()));
    new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_Tab, this, SLOT(nextContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageUp, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageDown, this, SLOT(nextContact()));

#ifdef Q_OS_MAC
    QMenuBar* globalMenu = Nexus::getInstance().globalMenuBar;
    QAction* windowMenu = Nexus::getInstance().windowMenu->menuAction();
    QAction* viewMenu = Nexus::getInstance().viewMenu->menuAction();
    QAction* frontAction = Nexus::getInstance().frontAction;

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
    connect(logoutAction, &QAction::triggered, [this]()
    {
        Nexus::getInstance().showLogin();
    });

    editMenu = globalMenu->insertMenu(viewMenu, new QMenu(this));
    editMenu->menu()->addSeparator();

    viewMenu->menu()->insertMenu(Nexus::getInstance().fullscreenAction, filterMenu);

    viewMenu->menu()->insertSeparator(Nexus::getInstance().fullscreenAction);

    contactMenu = globalMenu->insertMenu(windowMenu, new QMenu(this));

    addContactAction = contactMenu->menu()->addAction(QString());
    connect(addContactAction, &QAction::triggered, this, &Widget::onAddClicked);

    nextConversationAction = new QAction(this);
    Nexus::getInstance().windowMenu->insertAction(frontAction, nextConversationAction);
    nextConversationAction->setShortcut(QKeySequence::SelectNextPage);
    connect(nextConversationAction, &QAction::triggered, [this]()
    {
        if (ContentDialog::current() == QApplication::activeWindow())
            ContentDialog::current()->cycleContacts(true);
        else if (QApplication::activeWindow() == this)
            cycleContacts(true);
    });

    previousConversationAction = new QAction(this);
    Nexus::getInstance().windowMenu->insertAction(frontAction, previousConversationAction);
    previousConversationAction->setShortcut(QKeySequence::SelectPreviousPage);
    connect(previousConversationAction, &QAction::triggered, [this]
    {
        if (ContentDialog::current() == QApplication::activeWindow())
            ContentDialog::current()->cycleContacts(false);
        else if (QApplication::activeWindow() == this)
            cycleContacts(false);
    });

    windowMenu->menu()->insertSeparator(frontAction);

    QAction* preferencesAction = viewMenu->menu()->addAction(QString());
    preferencesAction->setMenuRole(QAction::PreferencesRole);
    connect(preferencesAction, &QAction::triggered, this, &Widget::onShowSettings);

    QAction* aboutAction = viewMenu->menu()->addAction(QString());
    aboutAction->setMenuRole(QAction::AboutRole);
    connect(aboutAction, &QAction::triggered, [this]()
    {
        onShowSettings();
        settingsWidget->showAbout();
    });

    QMenu* dockChangeStatusMenu = new QMenu(tr("Status"), this);
    dockChangeStatusMenu->addAction(statusOnline);
    statusOnline->setIconVisibleInMenu(true);
    dockChangeStatusMenu->addSeparator();
    dockChangeStatusMenu->addAction(statusAway);
    dockChangeStatusMenu->addAction(statusBusy);
    Nexus::getInstance().dockMenu->addAction(dockChangeStatusMenu->menuAction());

    connect(this, &Widget::windowStateChanged, &Nexus::getInstance(), &Nexus::onWindowStateChanged);
#endif

    //restore window state
    restoreGeometry(Settings::getInstance().getWindowGeometry());
    restoreState(Settings::getInstance().getWindowState());
    if (!mainSplitter->restoreState(Settings::getInstance().getSplitterState()))
    {
        // Set the status panel (friendlist) to a reasonnable width by default/on first start
        constexpr int spWidthPc = 33;
        mainSplitter->resize(size());
        QList<int> sizes = mainSplitter->sizes();
        sizes[0] = mainSplitter->width()*spWidthPc/100;
        sizes[1] = mainSplitter->width() - sizes[0];
        mainSplitter->setSizes(sizes);
    }

#if (AUTOUPDATE_ENABLED)
    if (Settings::getInstance().getCheckUpdates())
        AutoUpdater::checkUpdatesAsyncInteractive();
#endif

    friendRequestsButton = nullptr;
    groupInvitesButton = nullptr;
    unreadGroupInvites = 0;

    // settings
    const Settings& s = Settings::getInstance();
    connect(&s, &Settings::showSystemTrayChanged,
            this, &Widget::onSetShowSystemTray);
    connect(&s, &Settings::separateWindowChanged,
            this, &Widget::onSeparateWindowChanged);
    connect(&s, &Settings::groupchatPositionChanged,
            contactListWidget, &FriendListWidget::onGroupchatPositionChanged);

    retranslateUi();
    Translator::registerHandler(std::bind(&Widget::retranslateUi, this), this);

    if (!s.getSeparateWindow())
        onAddClicked();

    if (s.getShowSystemTray())
        hide();

#ifdef Q_OS_MAC
    Nexus::getInstance().updateWindows();
#endif
}

bool Widget::eventFilter(QObject *obj, QEvent *event)
{
    QWindowStateChangeEvent *ce = nullptr;
    Qt::WindowStates state = windowState();

    switch (event->type())
    {
    case QEvent::Close:
        // It's needed if user enable `Close to tray`
        wasMaximized = state.testFlag(Qt::WindowMaximized);
        break;

    case QEvent::WindowStateChange:
        ce = static_cast<QWindowStateChangeEvent*>(event);
        if (state.testFlag(Qt::WindowMinimized) && obj)
            wasMaximized = ce->oldState().testFlag(Qt::WindowMaximized);

#ifdef Q_OS_MAC
        emit windowStateChanged(state);
#endif
        break;
    default:
        break;
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
        status = QStringLiteral("event");
    }
    else
    {
        status = statusButton->property("status").toString();
        if (!status.length())
            status = QStringLiteral("offline");
    }

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

    if (!checkedHasThemeIcon)
    {
        hasThemeIconBug = QIcon::hasThemeIcon("qtox-asjkdfhawjkeghdfjgh");
        checkedHasThemeIcon = true;

        if (hasThemeIconBug)
        {
            qDebug() << "Detected buggy QIcon::hasThemeIcon. Icon overrides from theme will be ignored.";
        }
    }

    QIcon ico;
    if (!hasThemeIconBug && QIcon::hasThemeIcon("qtox-" + status))
    {
        ico = QIcon::fromTheme("qtox-" + status);
    }
    else
    {
        QString color = Settings::getInstance().getLightTrayIcon() ? "light" : "dark";
        QString path = ":/img/taskbar/" + color + "/taskbar_" + status + ".svg";
        QSvgRenderer renderer(path);

        // Prepare a QImage with desired characteritisc
        QImage image = QImage(250, 250, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        renderer.render(&painter);
        ico = QIcon(QPixmap::fromImage(image));
    }

    setWindowIcon(ico);
    if (icon)
        icon->setIcon(ico);
}

Widget::~Widget()
{
    QWidgetList windowList = QApplication::topLevelWidgets();

    for (QWidget* window : windowList)
    {
        if (window != this)
            window->close();
    }

    Translator::unregister(this);
    AutoUpdater::abortUpdates();
    if (icon)
        icon->hide();

    delete icon;
    delete timer;
    delete offlineMsgTimer;

    FriendList::clear();
    GroupList::clear();
    delete trayMenu;
    instance = nullptr;
}

/**
 * @brief Returns the singleton instance.
 */
Widget* Widget::getInstance()
{
    if (!instance)
        instance = new Widget();

    return instance;
}

QSize Widget::minimumSizeHint() const
{
    QSize size(300, 480);

    if (contentWidget)
        size.rwidth() = mainSplitter->minimumWidth();
    else
        size.rwidth() = tooliconsZone->minimumSize().width();

    return size;
}

/**
 * @brief Switches to the About settings page.
 */
void Widget::showUpdateDownloadProgress()
{
    onShowSettings();
    settingsWidget->showAbout();
}

void Widget::moveEvent(QMoveEvent *event)
{
    if (event->pos() != event->oldPos())
    {
        if (!Settings::getInstance().getSeparateWindow())
        {
            arrangeContent();
        }

        saveWindowGeometry();
    }

    QWidget::moveEvent(event);
}

void Widget::closeEvent(QCloseEvent *event)
{
    if (Settings::getInstance().getShowSystemTray() && Settings::getInstance().getCloseToTray())
    {
        QWidget::closeEvent(event);
    }
    else
    {
        if (autoAwayActive)
        {
            emit statusSet(Status::Online);
            autoAwayActive = false;
        }
        saveWindowGeometry();
        saveSplitterGeometry();
        QWidget::closeEvent(event);
        qApp->quit();
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
    if (event->size() != event->oldSize())
        saveWindowGeometry();

    QMainWindow::resizeEvent(event);
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
    statusButton->setEnabled(true);
    emit statusSet(Nexus::getCore()->getStatus());
}

void Widget::onDisconnected()
{
    statusButton->setEnabled(false);
    emit statusSet(Status::Offline);
}

void Widget::onFailedToStartCore()
{
    QMessageBox critical(this);
    critical.setText(tr("toxcore failed to start, the application will terminate after you close this message."));
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    qApp->quit();
}

void Widget::onBadProxyCore()
{
    Settings::getInstance().setProxyType(Settings::ProxyType::ptNone);
    QMessageBox critical(this);
    critical.setText(tr("toxcore failed to start with your proxy settings. qTox cannot run; please modify your "
               "settings and restart.", "popup text"));
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    onShowSettings();
}

void Widget::onStatusSet(Status status)
{
    statusButton->setProperty("status", getStatusTitle(status));
    statusButton->setIcon(prepareIcon(getStatusIconPath(status), icon_size, icon_size));
    updateIcons();
}

void Widget::onSeparateWindowChanged(bool separate)
{
    if (separate)
    {
        showContentWidget(contentWidget);
    }
    else
    {
        QWindow* focusWindow = QApplication::focusWindow();
        for (QWindow* w : QApplication::topLevelWindows())
        {
            if (w != focusWindow &&
                w->objectName() != QStringLiteral("MainWindowWindow"))
            {
                w->close();
            }
        }

        QWidget* tlw = QApplication::activeWindow();
        showContentWidget(tlw, tlw->windowTitle());
    }

}

void Widget::setWindowTitle(const QString& title)
{
    if (title.isEmpty())
    {
        QMainWindow::setWindowTitle(QApplication::applicationName());
    }
    else
    {
        QString tmp = title;
        /// <[^>]*> Regexp to remove HTML tags, in case someone used them in title
        QMainWindow::setWindowTitle(QApplication::applicationName() + QStringLiteral(" - ") + tmp.remove(QRegExp("<[^>]*>")));
    }
}

void Widget::forceShow()
{
    // restore minimized windows
    for (QWidget* w : QApplication::topLevelWidgets())
    {
        if (w->isWindow())
        {
            if (w->isMinimized())
                w->showNormal();
            else
                w->show();
        }
    }

    activateWindow();
}

void Widget::onAddClicked()
{
    if (!addFriendForm)
    {
        addFriendForm = new AddFriendForm(this);

        connect(addFriendForm, &AddFriendForm::friendRequested,
                this, &Widget::friendRequested);
        connect(addFriendForm, &AddFriendForm::friendRequested,
                this, &Widget::friendRequestsUpdate);
        connect(addFriendForm, &AddFriendForm::friendRequestsSeen,
                this, &Widget::friendRequestsUpdate);
        connect(addFriendForm, &AddFriendForm::friendRequestAccepted,
                this, &Widget::friendRequestAccepted);
    }

    showContentWidget(addFriendForm, fromDialogType(DialogType::AddDialog),
             ActiveToolMenuButton::AddButton);
}

void Widget::onGroupClicked()
{
    if (!groupInviteForm)
    {
        groupInviteForm = new GroupInviteForm(this);

        connect(groupInviteForm, &GroupInviteForm::groupCreate,
                Core::getInstance(), &Core::createGroup);
        connect(groupInviteForm, &GroupInviteForm::groupInvitesSeen,
                this, &Widget::groupInvitesClear);
        connect(groupInviteForm, &GroupInviteForm::groupInviteAccepted,
                this, &Widget::onGroupInviteAccepted);
    }

    showContentWidget(groupInviteForm, fromDialogType(DialogType::GroupDialog),
             ActiveToolMenuButton::GroupButton);
}

void Widget::onTransferClicked()
{
    if (!filesForm)
    {
        filesForm = new FilesForm(this);

        Core* core = Nexus::getCore();
        connect(core, &Core::fileDownloadFinished,
                filesForm, &FilesForm::onFileDownloadComplete);
        connect(core, &Core::fileUploadFinished,
                filesForm, &FilesForm::onFileUploadComplete);
    }

    showContentWidget(filesForm, fromDialogType(DialogType::TransferDialog),
             ActiveToolMenuButton::TransferButton);
}

void Widget::confirmExecutableOpen(const QFileInfo &file)
{
    static const QStringList dangerousExtensions = { "app", "bat", "com", "cpl",
                                                     "dmg", "exe", "hta", "jar",
                                                     "js", "jse", "lnk", "msc",
                                                     "msh", "msh1", "msh1xml",
                                                     "msh2", "msh2xml",
                                                     "mshxml", "msi", "msp",
                                                     "pif", "ps1", "ps1xml",
                                                     "ps2", "ps2xml", "psc1",
                                                     "psc2", "py", "reg", "scf",
                                                     "sh", "src", "vb", "vbe",
                                                     "vbs", "ws", "wsc", "wsf",
                                                     "wsh" };

    if (dangerousExtensions.contains(file.suffix()))
    {
        if (!GUI::askQuestion(tr("Executable file", "popup title"),
                              tr("You have asked qTox to open an executable"
                                 " file. Executable files can potentially"
                                 " damage your computer. Are you sure want to"
                                 " open this file?", "popup text"),
                              false, true))
            return;

        // The user wants to run this file, so make it executable and run it
        QFile(file.filePath()).setPermissions(file.permissions() |
                                              QFile::ExeOwner |
                                              QFile::ExeUser |
                                              QFile::ExeGroup |
                                              QFile::ExeOther);
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(file.filePath()));
}

void Widget::onIconClick(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        if (isHidden() || isMinimized())
        {
            if (wasMaximized)
                showMaximized();
            else
                showNormal();

            activateWindow();
        }
        else if (!isActiveWindow())
        {
            activateWindow();
        }
        else
        {
            wasMaximized = isMaximized();
            hide();
        }
    }
    else if (reason == QSystemTrayIcon::Unknown)
    {
        if (isHidden())
            forceShow();
    }
}

void Widget::onShowSettings()
{
    if (!settingsWidget)
    {
        settingsWidget = new SettingsWidget(this);
        settingsWidget->setWindowIcon(QIcon(":/img/icons/qtox.svg"));
    }

    showContentWidget(settingsWidget, fromDialogType(DialogType::SettingDialog),
             ActiveToolMenuButton::SettingButton);
}

void Widget::showProfile()
{
    if (!profileForm)
    {
        profileForm = new ContentWidget(this);
        ProfileForm* profileUi = new ProfileForm;
        profileForm->setupLayout(profileUi->getHeadWidget(), profileUi);
    }

    showContentWidget(profileForm, fromDialogType(DialogType::ProfileDialog),
             ActiveToolMenuButton::None);
}

void Widget::setUsername(const QString& username)
{
    if (username.isEmpty())
    {
        nameLabel->setText(tr("Your name"));
        nameLabel->setToolTip(tr("Your name"));
    }
    else
    {
        nameLabel->setText(username);
        nameLabel->setToolTip(Qt::convertFromPlainText(username, Qt::WhiteSpaceNormal)); // for overlength names
    }

    QString sanename = username;
    sanename.remove(QRegExp("[\\t\\n\\v\\f\\r\\x0000]"));
             nameMention = QRegExp("\\b" + QRegExp::escape(username) + "\\b", Qt::CaseInsensitive);
    sanitizedNameMention = QRegExp("\\b" + QRegExp::escape(sanename) + "\\b", Qt::CaseInsensitive);
}

void Widget::onStatusMessageChanged(const QString& newStatusMessage)
{
    // Keep old status message until Core tells us to set it.
    Nexus::getCore()->setStatusMessage(newStatusMessage);
}

void Widget::setStatusMessage(const QString &statusMessage)
{
    if (statusMessage.isEmpty())
    {
        statusLabel->setText(tr("Your status"));
        statusLabel->setToolTip(tr("Your status"));
    }
    else
    {
        statusLabel->setText(statusMessage);
        statusLabel->setToolTip(Qt::convertFromPlainText(statusMessage, Qt::WhiteSpaceNormal)); // for overlength messsages
    }
}

void Widget::reloadHistory()
{
    for (auto f : FriendList::getAllFriends())
    {
        f->loadHistory();
    }
}

void Widget::addFriend(int friendId, const QString &userId)
{
    Settings& s = Settings::getInstance();
    ToxId userToxId = ToxId(userId);
    Friend* newfriend = FriendList::addFriend(friendId, userToxId);
    ChatForm* friendForm = new ChatForm(newfriend);

    QString name = newfriend->getDisplayedName();
    FriendWidget *widget = new FriendWidget(friendId, name);
    
    friendWidgets[newfriend] = widget;

    newfriend->setFriendWidget(widget);
    newfriend->loadHistory();

    QDate activityDate = s.getFriendActivity(newfriend->getToxId());
    QDate chatDate = friendForm->getLatestDate();

    if (chatDate > activityDate && chatDate.isValid())
        s.setFriendActivity(newfriend->getToxId(), chatDate);

    contactListWidget->addFriendWidget(widget, Status::Offline, s.getFriendCircleID(newfriend->getToxId()));

    Core* core = Nexus::getCore();
    CoreAV* coreav = core->getAv();
    connect(newfriend, &Friend::displayedNameChanged, this, &Widget::onFriendDisplayChanged);
    connect(widget, &FriendWidget::chatroomWidgetClicked, this, &Widget::onChatroomWidgetClicked);
    connect(widget, &FriendWidget::chatroomWidgetClicked, friendForm, &ChatForm::focusInput);
    connect(widget, &FriendWidget::copyFriendIdToClipboard, this, &Widget::copyFriendIdToClipboard);
    connect(widget, SIGNAL(removeFriend(int)), this, SLOT(removeFriend(int)));
    connect(friendForm, &GenericChatForm::sendMessage, core, &Core::sendMessage);
    connect(friendForm, &GenericChatForm::sendAction, core, &Core::sendAction);
    connect(friendForm, &ChatForm::sendFile, core, &Core::sendFile);
    connect(friendForm, &ChatForm::aliasChanged, widget, &FriendWidget::setAlias);
    connect(core, &Core::fileReceiveRequested, friendForm, &ChatForm::onFileRecvRequest);
    connect(coreav, &CoreAV::avInvite, friendForm, &ChatForm::onAvInvite, Qt::BlockingQueuedConnection);
    connect(coreav, &CoreAV::avStart, friendForm, &ChatForm::onAvStart, Qt::BlockingQueuedConnection);
    connect(coreav, &CoreAV::avEnd, friendForm, &ChatForm::onAvEnd, Qt::BlockingQueuedConnection);
    connect(core, &Core::friendAvatarChanged, friendForm, &ChatForm::onAvatarChange);
    connect(core, &Core::friendAvatarRemoved, friendForm, &ChatForm::onAvatarRemoved);
    connect(core, &Core::friendAvatarChanged, widget, &FriendWidget::onAvatarChange);
    connect(core, &Core::friendAvatarRemoved, widget, &FriendWidget::onAvatarRemoved);

    // Try to get the avatar from the cache
    QPixmap avatar = Nexus::getProfile()->loadAvatar(userId);
    if (!avatar.isNull())
    {
        friendForm->onAvatarChange(friendId, avatar);
        widget->onAvatarChange(friendId, avatar);
    }

    FilterCriteria filter = getFilterCriteria();
    widget->search(searchContactText->text(), filterOffline(filter));
}

void Widget::addFriendFailed(const QString&, const QString& errorInfo)
{
    QString info = QString(tr("Couldn't request friendship"));
    if (!errorInfo.isEmpty()) {
        info = info + (QString(": ") + errorInfo);
    }

    QMessageBox::critical(0,"Error",info);
}

void Widget::onFriendshipChanged(int friendId)
{
    Friend* who = FriendList::findFriend(friendId);
    updateFriendActivity(who);
}

void Widget::onFriendStatusChanged(int friendId, Status status)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    bool isActualChange = f->getStatus() != status;

    FriendWidget *widget = friendWidgets[f];
    if (isActualChange)
    {
        if (f->getStatus() == Status::Offline)
            contactListWidget->moveWidget(widget, Status::Online);
        else if (status == Status::Offline)
            contactListWidget->moveWidget(widget, Status::Offline);
    }

    f->setStatus(status);
    widget->updateStatusLight();
    if (widget->isActive())
        setWindowTitle(widget->getTitle());

    ContentDialog::updateFriendStatus(friendId);
}

void Widget::onFriendStatusMessageChanged(int friendId, const QString& message)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    QString str = message; str.replace('\n', ' ');
    str.remove('\r');
    str.remove(QChar()); // null terminator...
    f->setStatusMessage(str);

    ContentDialog::updateFriendStatusMessage(friendId, message);
}

void Widget::onFriendUsernameChanged(int friendId, const QString& username)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    QString str = username; str.replace('\n', ' ');
    str.remove('\r');
    str.remove(QChar()); // null terminator...
    f->setName(str);
}

void Widget::onFriendDisplayChanged(FriendWidget *friendWidget, Status s)
{
    contactListWidget->moveWidget(friendWidget, s);
    FilterCriteria filter = getFilterCriteria();
    switch (s)
    {
    case Status::Offline:
        friendWidget->searchName(searchContactText->text(),
                                 filterOffline(filter));
        break;
    case Status::Online:
    case Status::Busy:
    case Status::Away:
        friendWidget->searchName(searchContactText->text(),
                                 filterOnline(filter));
        break;
    }
}

void Widget::onChatroomWidgetClicked(GenericChatroomWidget *widget, bool group)
{
    widget->resetEventFlags();
    widget->updateStatusLight();

    if (widget->chatFormIsSet(true) && !group)
        return;

    GenericChatForm* chatWidget = nullptr;
    Friend* frnd = widget->getFriend();

    if (frnd)
    {
        chatWidget = new ChatForm(frnd);
    }
    else
    {
        Group* g = widget->getGroup();
        chatWidget = g->getChatForm();
    }

    if (chatWidget)
    {
        activeChat = chatWidget;
        widget->setAsActiveChatroom();
        frnd->setEventFlag(false);
    }

    showContentWidget(chatWidget, chatWidget->windowTitle());
}

void Widget::onFriendMessageReceived(int friendId, const QString& message, bool isAction)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    QDateTime timestamp = QDateTime::currentDateTime();
    Profile* profile = Nexus::getProfile();
    if (profile->isHistoryEnabled())
        profile->getHistory()->addNewMessage(f->getToxId().publicKey, isAction ? ChatForm::ACTION_PREFIX + f->getDisplayedName() + " " + message : message,
                                               f->getToxId().publicKey, timestamp, true, f->getDisplayedName());

    newFriendMessageAlert(friendId);
}

void Widget::addFriendDialog(Friend *frnd, ContentDialog *dialog)
{
    FriendWidget *widget = friendWidgets[frnd];
    FriendWidget* friendWidget = dialog->addFriend(frnd->getFriendId(),
                                                   frnd->getDisplayedName());

    friendWidget->setStatusMsg(widget->getStatusMsg());

    connect(friendWidget, SIGNAL(removeFriend(int)), this, SLOT(removeFriend(int)));
    connect(friendWidget, SIGNAL(copyFriendIdToClipboard(int)), this, SLOT(copyFriendIdToClipboard(int)));
    connect(friendWidget, &FriendWidget::chatroomWidgetClicked, this, [=](GenericChatroomWidget *w, bool group)
    {
        Q_UNUSED(w);
        emit widget->chatroomWidgetClicked(widget, group);
    });
    emit widget->chatroomWidgetClicked(widget, false);

    connect(Core::getInstance(), &Core::friendAvatarChanged, friendWidget, &FriendWidget::onAvatarChange);
    connect(Core::getInstance(), &Core::friendAvatarRemoved, friendWidget, &FriendWidget::onAvatarRemoved);

    QPixmap avatar = Nexus::getProfile()->loadAvatar(frnd->getToxId().toString());
    if (!avatar.isNull())
        friendWidget->onAvatarChange(frnd->getFriendId(), avatar);
}

void Widget::addGroupDialog(Group *group, ContentDialog *dialog)
{
    ContentDialog *contentDialog = ContentDialog::getGroupDialog(group->getGroupId());
    bool separate = Settings::getInstance().getSeparateWindow();
    GroupWidget *widget = group->getGroupWidget();
    bool isCurrent = activeChat == group->getChatForm();
    if (!contentDialog && !separate && isCurrent)
        onAddClicked();

    GroupWidget* groupWidget = dialog->addGroup(group->getGroupId(), group->getName());
    connect(groupWidget, SIGNAL(removeGroup(int)), this, SLOT(removeGroup(int)));
    connect(groupWidget, SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), group->getChatForm(), SLOT(focusInput()));
    connect(groupWidget, &FriendWidget::chatroomWidgetClicked, this, [=](GenericChatroomWidget *w, bool group)
    {
        Q_UNUSED(w);
        emit widget->chatroomWidgetClicked(widget, group);
    });
    emit widget->chatroomWidgetClicked(widget, false);
}

bool Widget::newFriendMessageAlert(int friendId, bool sound)
{
    bool hasActive;
    QWidget* currentWindow;
    ContentDialog* contentDialog = ContentDialog::getFriendDialog(friendId);
    Friend* f = FriendList::findFriend(friendId);

    if (contentDialog != nullptr)
    {
        currentWindow = contentDialog->window();
        hasActive = ContentDialog::isFriendWidgetActive(friendId);
    }
    else
    {
        if (Settings::getInstance().getSeparateWindow() &&
            Settings::getInstance().getShowWindow())
        {
            if (Settings::getInstance().getDontGroupWindows())
            {
                contentDialog = createContentDialog();
            }
            else
            {
                contentDialog = ContentDialog::current();
                if (!contentDialog)
                    contentDialog = createContentDialog();
            }

            addFriendDialog(f, contentDialog);
            currentWindow = contentDialog->window();
            hasActive = ContentDialog::isFriendWidgetActive(friendId);
        }
        else
        {
            currentWindow = window();
            hasActive = activeChat;
        }
    }

    if (newMessageAlert(currentWindow, hasActive, sound))
    {
        FriendWidget *widget = friendWidgets[f];
        f->setEventFlag(true);
        widget->updateStatusLight();
        friendList->trackWidget(widget);

        if (contentDialog == nullptr)
        {
            if (hasActive)
                setWindowTitle(widget->getTitle());
        }
        else
        {
            ContentDialog::updateFriendStatus(friendId);
        }

        return true;
    }

    return false;
}

bool Widget::newGroupMessageAlert(int groupId, bool notify)
{
    bool hasActive;
    QWidget* currentWindow;
    ContentDialog* contentDialog = ContentDialog::getGroupDialog(groupId);
    Group* g =  GroupList::findGroup(groupId);

    if (contentDialog != nullptr)
    {
        currentWindow = contentDialog->window();
        hasActive = ContentDialog::isGroupWidgetActive(groupId);
    }
    else
    {
        currentWindow = window();
        hasActive = g->getChatForm() == activeChat;
    }

    if (newMessageAlert(currentWindow, hasActive, true, notify))
    {
        g->setEventFlag(true);
        g->getGroupWidget()->updateStatusLight();

        if (contentDialog == nullptr)
        {
            if (hasActive)
                setWindowTitle(g->getGroupWidget()->getTitle());
        }
        else
        {
            ContentDialog::updateGroupStatus(groupId);
        }

        return true;
    }

    return false;
}

QString Widget::fromDialogType(DialogType type)
{
    switch (type)
    {
    case DialogType::AddDialog:
        return tr("Add friend");
    case DialogType::GroupDialog:
        return tr("Group invites");
    case DialogType::TransferDialog:
        return tr("File transfers");
    case DialogType::SettingDialog:
        return tr("Settings");
    case DialogType::ProfileDialog:
        return tr("Profile");
    }

    return QString();
}

bool Widget::newMessageAlert(QWidget* currentWindow, bool isActive, bool sound, bool notify)
{
    bool inactiveWindow = isMinimized() || !currentWindow->isActiveWindow();

    if (!inactiveWindow && isActive)
        return false;

    if (notify)
    {
        if (inactiveWindow)
        {
            QApplication::alert(currentWindow);
            eventFlag = true;
        }

        if (Settings::getInstance().getShowWindow())
        {
            currentWindow->show();
            if (inactiveWindow && Settings::getInstance().getShowInFront())
                currentWindow->activateWindow();
        }

        bool isBusy = Nexus::getCore()->getStatus() == Status::Busy;
        bool busySound = Settings::getInstance().getBusySound();
        bool notifySound = Settings::getInstance().getNotifySound();

        if (notifySound && sound && (!isBusy || busySound))
            Audio::getInstance().playMono16Sound(QStringLiteral(":/audio/notification.pcm"));
    }

    return true;
}

void Widget::onFriendRequestReceived(const QString& userId, const QString& message)
{
    if (!addFriendForm)
    {
        addFriendForm = new AddFriendForm(this);

        connect(addFriendForm, &AddFriendForm::friendRequested,
                this, &Widget::friendRequested);
        connect(addFriendForm, &AddFriendForm::friendRequested,
                this, &Widget::friendRequestsUpdate);
        connect(addFriendForm, &AddFriendForm::friendRequestsSeen,
                this, &Widget::friendRequestsUpdate);
        connect(addFriendForm, &AddFriendForm::friendRequestAccepted,
                this, &Widget::friendRequestAccepted);
    }

    if (addFriendForm->addFriendRequest(userId, message))
    {
        friendRequestsUpdate();
        newMessageAlert(window(), isActiveWindow(), true, true);
    }
}

void Widget::updateFriendActivity(Friend *frnd)
{
    QDate date = Settings::getInstance().getFriendActivity(frnd->getToxId());
    if (date != QDate::currentDate())
    {
        // Update old activity before after new one. Store old date first.
        QDate oldDate = Settings::getInstance().getFriendActivity(frnd->getToxId());
        Settings::getInstance().setFriendActivity(frnd->getToxId(), QDate::currentDate());
        contactListWidget->moveWidget(friendWidgets[frnd], frnd->getStatus());
        contactListWidget->updateActivityDate(oldDate);
    }
}

void Widget::removeFriend(Friend* f, bool fake)
{
    if (!fake)
    {
        RemoveFriendDialog ask(this, f);
        ask.exec();

        if (!ask.accepted())
               return;

        if (ask.removeHistory())
            Nexus::getProfile()->getHistory()->removeFriendHistory(f->getToxId().publicKey);
    }

    FriendWidget *widget = friendWidgets[f];
    widget->setAsInactiveChatroom();
    if (!activeChat)
        onAddClicked();

    contactListWidget->removeFriendWidget(widget);

    ContentDialog* lastDialog = ContentDialog::getFriendDialog(f->getFriendId());

    if (lastDialog != nullptr)
        lastDialog->removeFriend(f->getFriendId());

    FriendList::removeFriend(f->getFriendId(), fake);
    Nexus::getCore()->removeFriend(f->getFriendId(), fake);

    delete f;

    if (!Settings::getInstance().getSeparateWindow())
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

void Widget::onDialogShown(GenericChatroomWidget *widget)
{
    widget->resetEventFlags();
    widget->updateStatusLight();

    friendList->updateTracking(widget);
    resetIcon();
}

void Widget::onFriendDialogShown(Friend *f)
{
    onDialogShown(friendWidgets[f]);
}

void Widget::onGroupDialogShown(Group *g)
{
    onDialogShown(g->getGroupWidget());
}

ContentDialog* Widget::createContentDialog()
{
    ContentDialog* contentDialog = new ContentDialog(this);
    connect(contentDialog, &ContentDialog::friendDialogShown, this, &Widget::onFriendDialogShown);
    connect(contentDialog, &ContentDialog::groupDialogShown, this, &Widget::onGroupDialogShown);

#ifdef Q_OS_MAC
    connect(contentDialog, &ContentDialog::destroyed, &Nexus::getInstance(), &Nexus::updateWindowsClosed);
    connect(contentDialog, &ContentDialog::windowStateChanged, &Nexus::getInstance(), &Nexus::onWindowStateChanged);
    connect(contentDialog->windowHandle(), &QWindow::windowTitleChanged, &Nexus::getInstance(), &Nexus::updateWindows);
    Nexus::getInstance().updateWindows();
#endif
    return contentDialog;
}

ContentLayout* Widget::createContentDialog(DialogType type) const
{
    class Dialog : public ActivateDialog
    {
    public:
        explicit Dialog(DialogType type)
            : ActivateDialog(this)
            , type(type)
        {
            restoreGeometry(Settings::getInstance().getDialogSettingsGeometry());
            retranslateUi();
            Translator::registerHandler(std::bind(&Dialog::retranslateUi, this), this);

            connect(Core::getInstance(), &Core::usernameSet, this, &Dialog::retranslateUi);
        }

        ~Dialog()
        {
            Translator::unregister(this);
        }

    public slots:
        void retranslateUi()
        {
            setWindowTitle(Core::getInstance()->getUsername() + QStringLiteral(" - ") + Widget::fromDialogType(type));
            setWindowIcon(QIcon(":/img/icons/qtox.svg"));
        }

    protected:
        void resizeEvent(QResizeEvent* event) override
        {
            Settings::getInstance().setDialogSettingsGeometry(saveGeometry());
            QDialog::resizeEvent(event);
        }

        void moveEvent(QMoveEvent* event) override
        {
            Settings::getInstance().setDialogSettingsGeometry(saveGeometry());
            QDialog::moveEvent(event);
        }

    private:
        DialogType type;
    };

    Dialog* dialog = new Dialog(type);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    ContentLayout* contentLayoutDialog = new ContentLayout(dialog);

    dialog->setLayout(contentLayoutDialog);
    dialog->layout()->setMargin(0);
    dialog->layout()->setSpacing(0);
    dialog->setMinimumSize(720, 400);
    dialog->show();

#ifdef Q_OS_MAC
    connect(dialog, &Dialog::destroyed, &Nexus::getInstance(), &Nexus::updateWindowsClosed);
    connect(dialog, &ActivateDialog::windowStateChanged, &Nexus::getInstance(), &Nexus::updateWindowsStates);
    connect(dialog->windowHandle(), &QWindow::windowTitleChanged, &Nexus::getInstance(), &Nexus::updateWindows);
    Nexus::getInstance().updateWindows();
#endif

    return contentLayoutDialog;
}

void Widget::copyFriendIdToClipboard(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    if (f != nullptr)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(Nexus::getCore()->getFriendAddress(f->getFriendId()), QClipboard::Clipboard);
    }
}

void Widget::onGroupInviteReceived(int32_t friendId, uint8_t type, QByteArray invite)
{
    updateFriendActivity(FriendList::findFriend(friendId));

    if (type == TOX_GROUPCHAT_TYPE_TEXT || type == TOX_GROUPCHAT_TYPE_AV)
    {
        ++unreadGroupInvites;
        groupInvitesUpdate();
        newMessageAlert(window(), isActiveWindow(), true, true);

        if (!groupInviteForm)
        {
            groupInviteForm = new GroupInviteForm(this);

            connect(groupInviteForm, &GroupInviteForm::groupCreate,
                    Core::getInstance(), &Core::createGroup);
            connect(groupInviteForm, &GroupInviteForm::groupInvitesSeen,
                    this, &Widget::groupInvitesClear);
            connect(groupInviteForm, &GroupInviteForm::groupInviteAccepted,
                    this, &Widget::onGroupInviteAccepted);
        }

        groupInviteForm->addGroupInvite(friendId, type, invite);
    }
    else
    {
        qWarning() << "onGroupInviteReceived: Unknown groupchat type:"<<type;
        return;
    }
}

void Widget::onGroupInviteAccepted(int32_t friendId, uint8_t type, QByteArray invite)
{
    int groupId = Nexus::getCore()->joinGroupchat(friendId, type, (uint8_t*)invite.data(), invite.length());
    if (groupId < 0)
    {
        qWarning() << "onGroupInviteAccepted: Unable to accept group invite";
        return;
    }
}

void Widget::onGroupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    ToxId author = Core::getInstance()->getGroupPeerToxId(groupnumber, peernumber);

    bool targeted = !author.isSelf() && (message.contains(nameMention) || message.contains(sanitizedNameMention));
    if (targeted && !isAction)
        g->getChatForm()->addAlertMessage(author, message, QDateTime::currentDateTime());
    else
        g->getChatForm()->addMessage(author, message, isAction, QDateTime::currentDateTime(), true);

    newGroupMessageAlert(g->getGroupId(), targeted || Settings::getInstance().getGroupAlwaysNotify());
}

void Widget::onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t Change)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
    {
        qDebug() << "onGroupNamelistChanged: Group "<<groupnumber<<" not found, creating it";
        g = createGroup(groupnumber);
        if (!g)
            return;
    }


    TOX_CHAT_CHANGE change = static_cast<TOX_CHAT_CHANGE>(Change);
    if (change == TOX_CHAT_CHANGE_PEER_ADD)
    {
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
        QString name = Nexus::getCore()->getGroupPeerName(groupnumber, peernumber);
        if (name.isEmpty())
            name = tr("<Empty>", "Placeholder when someone's name in a group chat is empty");

        g->updatePeer(peernumber, name);
    }
}

void Widget::onGroupTitleChanged(int groupnumber, const QString& author, const QString& title)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    if (!author.isEmpty())
        g->getChatForm()->addSystemInfoMessage(tr("%1 has set the title to %2").arg(author, title), ChatMessage::INFO, QDateTime::currentDateTime());

    contactListWidget->renameGroupWidget(g->getGroupWidget(), title);
    g->setName(title);
    FilterCriteria filter = getFilterCriteria();
    g->getGroupWidget()->searchName(searchContactText->text(), filterGroups(filter));
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
    if (g->getChatForm() == activeChat)
        onAddClicked();

    GroupList::removeGroup(g->getGroupId(), fake);

    ContentDialog* contentDialog = ContentDialog::getGroupDialog(g->getGroupId());

    if (contentDialog != nullptr)
        contentDialog->removeGroup(g->getGroupId());

    Nexus::getCore()->removeGroup(g->getGroupId(), fake);
    delete g;

    if (!Settings::getInstance().getSeparateWindow())
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
        qWarning() << "Group already exists";
        return g;
    }

    Core* core = Nexus::getCore();

    if (!core)
    {
        qWarning() << "Can't create group. Core does not exist";
        return nullptr;
    }

    QString groupName = tr("Groupchat #%1").arg(groupId);
    CoreAV* coreAv = core->getAv();

    if (!coreAv)
    {
        qWarning() << "Can't create group. CoreAv does not exist";
        return nullptr;
    }

    bool enabled = coreAv->isGroupAvEnabled(groupId);
    Group* newgroup = GroupList::addGroup(groupId, groupName, enabled);

    contactListWidget->addGroupWidget(newgroup->getGroupWidget());
    newgroup->getGroupWidget()->updateStatusLight();
    contactListWidget->activateWindow();

    connect(newgroup->getGroupWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*,bool)), this, SLOT(onChatroomWidgetClicked(GenericChatroomWidget*,bool)));
    connect(newgroup->getGroupWidget(), SIGNAL(removeGroup(int)), this, SLOT(removeGroup(int)));
    connect(newgroup->getGroupWidget(), SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), newgroup->getChatForm(), SLOT(focusInput()));
    connect(newgroup->getChatForm(), &GroupChatForm::sendMessage, core, &Core::sendGroupMessage);
    connect(newgroup->getChatForm(), &GroupChatForm::sendAction, core, &Core::sendGroupAction);
    connect(newgroup->getChatForm(), &GroupChatForm::groupTitleChanged, core, &Core::changeGroupTitle);

    FilterCriteria filter = getFilterCriteria();
    newgroup->getGroupWidget()->searchName(searchContactText->text(), filterGroups(filter));

    return newgroup;
}

void Widget::onEmptyGroupCreated(int groupId)
{
    Group* group = createGroup(groupId);
    if (!group)
        return;

    // Only rename group if groups are visible.
    if (Widget::getInstance()->groupsVisible())
        group->getGroupWidget()->editName();
}

/**
 * @brief Used to reset the blinking icon.
 */
void Widget::resetIcon() {
    eventIcon = false;
    eventFlag = false;
    updateIcons();
}

bool Widget::event(QEvent * e)
{
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
        focusChatInput();
        break;
    case QEvent::Paint:
        friendList->updateVisualTracking();
        break;
    case QEvent::WindowActivate:
        if (eventFlag)
            resetIcon();

        focusChatInput();

#ifdef Q_OS_MAC
        emit windowStateChanged(windowState());

    case QEvent::WindowStateChange:
        Nexus::getInstance().updateWindowsStates();
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
    uint32_t autoAwayTime = Settings::getInstance().getAutoAwayTime() * 60 * 1000;

    if (statusButton->property("status").toString() == "online")
    {
        if (autoAwayTime && Platform::getIdleTime() >= autoAwayTime)
        {
            qDebug() << "auto away activated at" << QTime::currentTime().toString();
            emit statusSet(Status::Away);
            autoAwayActive = true;
        }
    }
    else if (statusButton->property("status").toString() == "away")
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
            icon = new SystemTrayIcon();
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

            // don't activate qTox widget on tray icon click in Unity backend (see #3419)
            if (icon->backend() != SystrayBackendType::Unity)
                connect(icon, &SystemTrayIcon::activated, this, &Widget::onIconClick);

            if (Settings::getInstance().getShowSystemTray())
            {
                icon->show();
                setHidden(Settings::getInstance().getAutostartInTray());
            }
            else
            {
                show();
            }

#ifdef Q_OS_MAC
            qt_mac_set_dock_menu(Nexus::getInstance().dockMenu);
#endif
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
    if (!statusButton->isEnabled())
        return;

    Nexus::getCore()->setStatus(Status::Online);
}

void Widget::setStatusAway()
{
    if (!statusButton->isEnabled())
        return;

    Nexus::getCore()->setStatus(Status::Away);
}

void Widget::setStatusBusy()
{
    if (!statusButton->isEnabled())
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
    Settings::getInstance().setSplitterState(mainSplitter->saveState());
}

void Widget::onSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);
    saveSplitterGeometry();
}

void Widget::cycleContacts(bool forward)
{
    // TODO: implementation
}

bool Widget::filterGroups(FilterCriteria filter)
{
    switch (filter)
    {
    case FilterCriteria::Offline:
    case FilterCriteria::Friends:
        return true;

    case FilterCriteria::Online:
    case FilterCriteria::Groups:
    case FilterCriteria::All:
        return false;
    }

    return false;
}

bool Widget::filterOffline(FilterCriteria filter)
{
    switch (filter)
    {
    case FilterCriteria::Online:
    case FilterCriteria::Groups:
        return true;

    case FilterCriteria::Offline:
    case FilterCriteria::Friends:
    case FilterCriteria::All:
        return false;
    }

    return false;
}

bool Widget::filterOnline(FilterCriteria filter)
{
    switch (filter)
    {
        case FilterCriteria::Offline:
        case FilterCriteria::Groups:
            return true;

    case FilterCriteria::Online:
    case FilterCriteria::Friends:
    case FilterCriteria::All:
            return false;
    }

    return false;
}

void Widget::processOfflineMsgs()
{
    if (OfflineMsgEngine::globalMutex.tryLock())
    {
        QList<Friend*> frnds = FriendList::getAllFriends();
        for (Friend* f : frnds)
        {
            f->deliverOfflineMsgs();
        }

        OfflineMsgEngine::globalMutex.unlock();
    }
}

void Widget::clearAllReceipts()
{
    QList<Friend*> frnds = FriendList::getAllFriends();
    for (Friend* f : frnds)
    {
        f->clearOfflineReceipts();
    }
}

void Widget::reloadTheme()
{
    QString statusPanelStyle = Style::getStylesheet(":/ui/window/statusPanel.css");
    tooliconsZone->setStyleSheet(Style::getStylesheet(":/ui/tooliconsZone/tooliconsZone.css"));
    statusPanel->setStyleSheet(statusPanelStyle);
    statusHead->setStyleSheet(statusPanelStyle);
    friendList->setStyleSheet(Style::getStylesheet(":/ui/friendList/friendList.css"));
    statusButton->setStyleSheet(Style::getStylesheet(":/ui/statusButton/statusButton.css"));
    contactListWidget->reDraw();

    for (Friend* f : FriendList::getAllFriends())
        friendWidgets[f]->reloadTheme();

    for (Group* g : GroupList::getAllGroups())
        g->getGroupWidget()->reloadTheme();
}

void Widget::nextContact()
{
    cycleContacts(true);
}

void Widget::previousContact()
{
    cycleContacts(false);
}

QString Widget::getStatusIconPath(Status status)
{
    switch (status)
    {
    case Status::Online:
        return ":/img/status/dot_online.svg";
    case Status::Away:
        return ":/img/status/dot_away.svg";
    case Status::Busy:
        return ":/img/status/dot_busy.svg";
    case Status::Offline:
        return ":/img/status/dot_offline.svg";
    }

    return QString();
}

inline QIcon Widget::prepareIcon(QString path, int w, int h)
{
#ifdef Q_OS_LINUX

    QString desktop = getenv("XDG_CURRENT_DESKTOP");
    if (desktop.isEmpty())
    {
        desktop = getenv("DESKTOP_SESSION");
    }
    desktop = desktop.toLower();

    if (desktop == "xfce" || desktop.contains("gnome") || desktop == "mate" || desktop == "x-cinnamon")
    {
        if (w > 0 && h > 0)
        {
            QSvgRenderer renderer(path);

            QPixmap pm(w, h);
            pm.fill(Qt::transparent);
            QPainter painter(&pm);
            renderer.render(&painter, pm.rect());

            return QIcon(pm);
        }
    }
#endif
    return QIcon(path);
}

QPixmap Widget::getStatusIconPixmap(QString path, uint32_t w, uint32_t h)
{
    QPixmap pix(w, h);
    pix.load(path);
    return pix;
}

QString Widget::getStatusTitle(Status status)
{
    switch (status)
    {
    case Status::Online:
        return QStringLiteral("online");
    case Status::Away:
        return QStringLiteral("away");
    case Status::Busy:
        return QStringLiteral("busy");
    case Status::Offline:
        return QStringLiteral("offline");
    }
}

Status Widget::getStatusFromString(QString status)
{
    if (status == QStringLiteral("online"))
        return Status::Online;
    else if (status == QStringLiteral("away"))
        return Status::Away;
    else if (status == QStringLiteral("busy"))
        return Status::Busy;
    else
        return Status::Offline;
}

void Widget::searchContacts()
{
    QString searchString = searchContactText->text();
    FilterCriteria filter = getFilterCriteria();

    contactListWidget->searchChatrooms(searchString, filterOnline(filter), filterOffline(filter), filterGroups(filter));

    updateFilterText();

    contactListWidget->reDraw();
}

void Widget::changeDisplayMode()
{
    filterDisplayGroup->setEnabled(false);

    if (filterDisplayGroup->checkedAction() == filterDisplayActivity)
        contactListWidget->setMode(FriendListWidget::Activity);
    else if (filterDisplayGroup->checkedAction() == filterDisplayName)
        contactListWidget->setMode(FriendListWidget::Name);

    searchContacts();
    filterDisplayGroup->setEnabled(true);

    updateFilterText();
}

void Widget::updateFilterText()
{
     searchContactFilterBox->setText(filterDisplayGroup->checkedAction()->text() + QStringLiteral(" | ") + filterGroup->checkedAction()->text());
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

void Widget::searchCircle(CircleWidget *circleWidget)
{
    FilterCriteria filter = getFilterCriteria();
    circleWidget->search(searchContactText->text(), true, filterOnline(filter), filterOffline(filter));
}

void Widget::searchItem(GenericChatItemWidget *chatItem, GenericChatItemWidget::ItemType type)
{
    bool hide;
    FilterCriteria filter = getFilterCriteria();
    switch (type)
    {
    case GenericChatItemWidget::GroupItem:
        hide = filterGroups(filter);
        break;
    case GenericChatItemWidget::FriendOfflineItem:
    case GenericChatItemWidget::FriendOnlineItem:
        hide = true;
        break;
    }

    chatItem->searchName(searchContactText->text(), hide);
}

bool Widget::groupsVisible() const
{
    FilterCriteria filter = getFilterCriteria();
    return !filterGroups(filter);
}

/**
 * @brief Arranges the content widgets depending on the position on screen.
 * @param[in] widget    the separate content widget, that will be placed
 *
 * In "separate window" mode, the widget is placed left or right next to the
 * main window. In "attached window" mode, the splitter position is flipped,
 * when crossing the virtual desktop's horizontal center.
 */
void Widget::arrangeContent(QWidget* widget)
{
    // TODO: use current screen center instead (don't forget to update docs)
    QRect dg = QApplication::desktop()->geometry();

    if (Settings::getInstance().getSeparateWindow())
    {
        if (widget)
        {
            QRect proposedGeom = frameGeometry();
            proposedGeom.adjust(0, 0, widget->width(), 0);
            const QRect& geom = geometry();
            int nx = proposedGeom.left() >= dg.center().x()
                     ? geom.left() - widget->width()
                     : geom.right();

            // move the active window attached to top-right / top-left
            widget->setGeometry(nx, geom.top(), widget->width(), height());
        }
    }
    else
    {
        QRect geom = frameGeometry();
        Qt::Edge arrangement = geom.left() > dg.center().x() ||
                               geom.right() >= dg.right()
                               ? Qt::RightEdge
                               : Qt::LeftEdge;

        if (arrangement != contentArrangement)
        {
            contentArrangement = arrangement;

            // reverse splitter positions
            int i = mainSplitter->count();
            while (--i >= 0)
            {
                mainSplitter->insertWidget(0, mainSplitter->widget(i));
            }
        }
    }
}

void Widget::friendListContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *createGroupAction = menu.addAction(tr("Create new group..."));
    QAction *addCircleAction = menu.addAction(tr("Add new circle..."));
    QAction *chosenAction = menu.exec(friendList->mapToGlobal(pos));

    if (chosenAction == addCircleAction)
        contactListWidget->addCircleWidget();
    else if (chosenAction == createGroupAction)
        Nexus::getCore()->createGroup();
}

void Widget::friendRequestsUpdate()
{
    unsigned int unreadFriendRequests = Settings::getInstance().getUnreadFriendRequests();

    if (unreadFriendRequests == 0)
    {
        delete friendRequestsButton;
        friendRequestsButton = nullptr;
    }
    else if (!friendRequestsButton)
    {
        friendRequestsButton = new QPushButton(this);
        friendRequestsButton->setObjectName("green");
        statusLayout->insertWidget(2, friendRequestsButton);

        connect(friendRequestsButton, &QPushButton::released, [this]()
        {
            onAddClicked();
            addFriendForm->setMode(AddFriendForm::Mode::FriendRequest);
        });
    }

    if (friendRequestsButton)
        friendRequestsButton->setText(tr("%n New Friend Request(s)", "", unreadFriendRequests));
}

void Widget::groupInvitesUpdate()
{
    if (unreadGroupInvites == 0)
    {
        delete groupInvitesButton;
        groupInvitesButton = nullptr;
    }
    else if (!groupInvitesButton)
    {
        groupInvitesButton = new QPushButton(this);
        groupInvitesButton->setObjectName("green");
        statusLayout->insertWidget(2, groupInvitesButton);

        connect(groupInvitesButton, &QPushButton::released, this, &Widget::onGroupClicked);
    }

    if (groupInvitesButton)
        groupInvitesButton->setText(tr("%n New Group Invite(s)", "", unreadGroupInvites));
}

void Widget::groupInvitesClear()
{
    unreadGroupInvites = 0;
    groupInvitesUpdate();
}

void Widget::setActiveToolMenuButton(ActiveToolMenuButton newActiveButton)
{
    addButton->setChecked(newActiveButton == ActiveToolMenuButton::AddButton);
    addButton->setDisabled(newActiveButton == ActiveToolMenuButton::AddButton);
    groupButton->setChecked(newActiveButton == ActiveToolMenuButton::GroupButton);
    groupButton->setDisabled(newActiveButton == ActiveToolMenuButton::GroupButton);
    transferButton->setChecked(newActiveButton == ActiveToolMenuButton::TransferButton);
    transferButton->setDisabled(newActiveButton == ActiveToolMenuButton::TransferButton);
    settingsButton->setChecked(newActiveButton == ActiveToolMenuButton::SettingButton);
    settingsButton->setDisabled(newActiveButton == ActiveToolMenuButton::SettingButton);
}

void Widget::retranslateUi()
{
    Ui::MainWindow::retranslateUi(this);

    Core* core = Nexus::getCore();
    setUsername(core->getUsername());
    setStatusMessage(core->getStatusMessage());

    filterDisplayName->setText(tr("By Name"));
    filterDisplayActivity->setText(tr("By Activity"));
    filterAllAction->setText(tr("All"));
    filterOnlineAction->setText(tr("Online"));
    filterOfflineAction->setText(tr("Offline"));
    filterFriendsAction->setText(tr("Friends"));
    filterGroupsAction->setText(tr("Groups"));
    searchContactText->setPlaceholderText(tr("Search Contacts"));
    updateFilterText();

    searchContactText->setPlaceholderText(tr("Search Contacts"));
    statusOnline->setText(tr("Online", "Button to set your status to 'Online'"));
    statusAway->setText(tr("Away", "Button to set your status to 'Away'"));
    statusBusy->setText(tr("Busy", "Button to set your status to 'Busy'"));
    actionLogout->setText(tr("Logout", "Tray action menu to logout user"));
    actionQuit->setText(tr("Exit", "Tray action menu to exit tox"));
    actionShow->setText(tr("Show", "Tray action menu to show qTox window"));

    if (!Settings::getInstance().getSeparateWindow())
        setWindowTitle(fromDialogType(DialogType::AddDialog));

    friendRequestsUpdate();
    groupInvitesUpdate();


#ifdef Q_OS_MAC
    Nexus::getInstance().retranslateUi();

    filterMenu->menuAction()->setText(tr("Filter..."));

    fileMenu->setText(tr("File"));
    editMenu->setText(tr("Edit"));
    contactMenu->setText(tr("Contacts"));
    changeStatusMenu->menuAction()->setText(tr("Change Status"));
    editProfileAction->setText(tr("Edit Profile"));
    logoutAction->setText(tr("Log out"));
    addContactAction->setText(tr("Add Contact..."));
    nextConversationAction->setText(tr("Next Conversation"));
    previousConversationAction->setText(tr("Previous Conversation"));
#endif
}

void Widget::focusChatInput()
{
    if (activeChat)
        activeChat->focusInput();
}

/**
 * @brief       Shows a widget "detached" or as "embedded widget".
 * @param[in]   contentWidget   the widget to show
 * @param[in]   title           the title in "embedded" mode
 * @param[in]   activeButton    the active tool button in "embedded" mode
 *
 * Depending on the mode, the widget is shown detached from the main window or
 * embedded in the main window's splitter.
 */
void Widget::showContentWidget(QWidget* widget, const QString& title,
                               Widget::ActiveToolMenuButton activeButton)
{
    Q_ASSERT(widget != this);

    QWidget* prevWidget = contentWidget;

    if (!widget)
    {
        setMinimumWidth(minimumSizeHint().width());
        return;
    }

    if (Settings::getInstance().getSeparateWindow())
    {
        contentWidget = nullptr;
        setWindowTitle(QString());
        setActiveToolMenuButton(ActiveToolMenuButton::None);

        // detach the content widget
        widget->setParent(nullptr);

        if (widget->isVisible())
        {
            if (widget->isMinimized())
            {
                widget->showNormal();
            }
            else
            {
                widget->raise();
                widget->activateWindow();
            }
        }
        else
        {
            widget->showNormal();

            setMinimumWidth(minimumSizeHint().width());

            if(prevWidget == widget)
                resize(width() - widget->width(), height());

            arrangeContent(widget);
        }
    }
    else
    {
        contentWidget = widget;

        if (prevWidget && prevWidget != contentWidget)
            prevWidget->close();

        setWindowTitle(title);
        setActiveToolMenuButton(activeButton);

        QList<int> sizes = mainSplitter->sizes();
        mainSplitter->insertWidget(1, contentWidget);
        sizes << contentWidget->minimumSizeHint().width();
        setMinimumWidth(minimumSizeHint().width() + sizes[1]);

        // restore splitter pos
        mainSplitter->setSizes(sizes);

        arrangeContent();
   }
}
