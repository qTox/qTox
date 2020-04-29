/*
    Copyright © 2014-2019 by The qTox Project Contributors

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


#include "nexus.h"
#include "persistence/settings.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/model/groupinvite.h"
#include "src/model/status.h"
#include "src/persistence/profile.h"
#include "src/widget/widget.h"
#include "video/camerasource.h"
#include "widget/gui.h"
#include "widget/loginscreen.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopWidget>
#include <QThread>
#include <cassert>
#include "audio/audio.h"
#include <vpx/vpx_image.h>

#ifdef Q_OS_MAC
#include <QActionGroup>
#include <QMenuBar>
#include <QSignalMapper>
#include <QWindow>
#endif

/**
 * @class Nexus
 *
 * This class is in charge of connecting various systems together
 * and forwarding signals appropriately to the right objects,
 * it is in charge of starting the GUI and the Core.
 */

Q_DECLARE_OPAQUE_POINTER(ToxAV*)

static Nexus* nexus{nullptr};

Nexus::Nexus(QObject* parent)
    : QObject(parent)
    , profile{nullptr}
    , widget{nullptr}
{}

Nexus::~Nexus()
{
    delete widget;
    widget = nullptr;
    delete profile;
    profile = nullptr;
    emit saveGlobal();
#ifdef Q_OS_MAC
    delete globalMenuBar;
#endif
}

/**
 * @brief Sets up invariants and calls showLogin
 *
 * Hides the login screen and shows the GUI for the given profile.
 * Will delete the current GUI, if it exists.
 */
void Nexus::start()
{
    qDebug() << "Starting up";

    // Setup the environment
    qRegisterMetaType<Status::Status>("Status::Status");
    qRegisterMetaType<vpx_image>("vpx_image");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<const int16_t*>("const int16_t*");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<QPixmap>("QPixmap");
    qRegisterMetaType<Profile*>("Profile*");
    qRegisterMetaType<ToxAV*>("ToxAV*");
    qRegisterMetaType<ToxFile>("ToxFile");
    qRegisterMetaType<ToxFile::FileDirection>("ToxFile::FileDirection");
    qRegisterMetaType<std::shared_ptr<VideoFrame>>("std::shared_ptr<VideoFrame>");
    qRegisterMetaType<ToxPk>("ToxPk");
    qRegisterMetaType<ToxId>("ToxId");
    qRegisterMetaType<ToxPk>("GroupId");
    qRegisterMetaType<ToxPk>("ContactId");
    qRegisterMetaType<GroupInvite>("GroupInvite");
    qRegisterMetaType<ReceiptNum>("ReceiptNum");
    qRegisterMetaType<RowId>("RowId");

    qApp->setQuitOnLastWindowClosed(false);

#ifdef Q_OS_MAC
    // TODO: still needed?
    globalMenuBar = new QMenuBar();
    dockMenu = new QMenu(globalMenuBar);

    viewMenu = globalMenuBar->addMenu(QString());

    windowMenu = globalMenuBar->addMenu(QString());
    globalMenuBar->addAction(windowMenu->menuAction());

    fullscreenAction = viewMenu->addAction(QString());
    fullscreenAction->setShortcut(QKeySequence::FullScreen);
    connect(fullscreenAction, &QAction::triggered, this, &Nexus::toggleFullscreen);

    minimizeAction = windowMenu->addAction(QString());
    minimizeAction->setShortcut(Qt::CTRL + Qt::Key_M);
    connect(minimizeAction, &QAction::triggered, [this]() {
        minimizeAction->setEnabled(false);
        QApplication::focusWindow()->showMinimized();
    });

    windowMenu->addSeparator();
    frontAction = windowMenu->addAction(QString());
    connect(frontAction, &QAction::triggered, this, &Nexus::bringAllToFront);

    QAction* quitAction = new QAction(globalMenuBar);
    quitAction->setMenuRole(QAction::QuitRole);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    retranslateUi();
#endif
    showMainGUI();
}

/**
 * @brief Hides the main GUI, delete the profile, and shows the login screen
 */
int Nexus::showLogin(const QString& profileName)
{
    delete widget;
    widget = nullptr;

    delete profile;
    profile = nullptr;

    LoginScreen loginScreen{profileName};
    connectLoginScreen(loginScreen);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // TODO(kriby): Move core out of profile
    // This is awkward because the core is in the profile
    // The connection order ensures profile will be ready for bootstrap for now
    connect(this, &Nexus::currentProfileChanged, this, &Nexus::bootstrapWithProfile);
    int returnval = loginScreen.exec();
    if (returnval == QDialog::Rejected) {
        // Kriby: This will terminate the main application loop, necessary until we refactor
        // away the split startup/return to login behavior.
        qApp->quit();
    }
    disconnect(this, &Nexus::currentProfileChanged, this, &Nexus::bootstrapWithProfile);
    return returnval;
}

void Nexus::bootstrapWithProfile(Profile* p)
{
    // kriby: This is a hack until a proper controller is written

    profile = p;

    if (profile) {
        audioControl = std::unique_ptr<IAudioControl>(Audio::makeAudio(*settings));
        assert(audioControl != nullptr);
        profile->getCore().getAv()->setAudio(*audioControl);
        start();
    }
}

void Nexus::setSettings(Settings* settings)
{
    if (this->settings) {
        QObject::disconnect(this, &Nexus::saveGlobal, this->settings, &Settings::saveGlobal);
    }
    this->settings = settings;
    if (this->settings) {
        QObject::connect(this, &Nexus::saveGlobal, this->settings, &Settings::saveGlobal);
    }
}

void Nexus::connectLoginScreen(const LoginScreen& loginScreen)
{
    // TODO(kriby): Move connect sequences to a controller class object instead

    // Nexus -> LoginScreen
    QObject::connect(this, &Nexus::profileLoaded, &loginScreen, &LoginScreen::onProfileLoaded);
    QObject::connect(this, &Nexus::profileLoadFailed, &loginScreen, &LoginScreen::onProfileLoadFailed);
    // LoginScreen -> Nexus
    QObject::connect(&loginScreen, &LoginScreen::createNewProfile, this, &Nexus::onCreateNewProfile);
    QObject::connect(&loginScreen, &LoginScreen::loadProfile, this, &Nexus::onLoadProfile);
    // LoginScreen -> Settings
    QObject::connect(&loginScreen, &LoginScreen::autoLoginChanged, settings, &Settings::setAutoLogin);
    QObject::connect(&loginScreen, &LoginScreen::autoLoginChanged, settings, &Settings::saveGlobal);
    // Settings -> LoginScreen
    QObject::connect(settings, &Settings::autoLoginChanged, &loginScreen,
                     &LoginScreen::onAutoLoginChanged);
}

void Nexus::showMainGUI()
{
    // TODO(kriby): Rewrite as view-model connect sequence only, add to a controller class object
    assert(profile);

    // Create GUI
    widget = new Widget(*profile, *audioControl);

    // Start GUI
    widget->init();
    GUI::getInstance();

    // Zetok protection
    // There are small instants on startup during which no
    // profile is loaded but the GUI could still receive events,
    // e.g. between two modal windows. Disable the GUI to prevent that.
    GUI::setEnabled(false);

    // Connections
    connect(profile, &Profile::selfAvatarChanged, widget, &Widget::onSelfAvatarLoaded);

    connect(profile, &Profile::coreChanged, widget, &Widget::onCoreChanged);

    connect(profile, &Profile::failedToStart, widget, &Widget::onFailedToStartCore,
            Qt::BlockingQueuedConnection);

    connect(profile, &Profile::badProxy, widget, &Widget::onBadProxyCore, Qt::BlockingQueuedConnection);

    profile->startCore();

    GUI::setEnabled(true);
}

/**
 * @brief Returns the singleton instance.
 */
Nexus& Nexus::getInstance()
{
    if (!nexus)
        nexus = new Nexus;

    return *nexus;
}

void Nexus::destroyInstance()
{
    delete nexus;
    nexus = nullptr;
}

/**
 * @brief Get core instance.
 * @return nullptr if not started, core instance otherwise.
 */
Core* Nexus::getCore()
{
    Nexus& nexus = getInstance();
    if (!nexus.profile)
        return nullptr;

    return &nexus.profile->getCore();
}

/**
 * @brief Get current user profile.
 * @return nullptr if not started, profile otherwise.
 */
Profile* Nexus::getProfile()
{
    return getInstance().profile;
}

/**
 * @brief Creates a new profile and replaces the current one.
 * @param name New username
 * @param pass New password
 */
void Nexus::onCreateNewProfile(const QString& name, const QString& pass)
{
    setProfile(Profile::createProfile(name, pass, *settings, parser));
    parser = nullptr; // only apply cmdline proxy settings once
}

/**
 * Loads an existing profile and replaces the current one.
 */
void Nexus::onLoadProfile(const QString& name, const QString& pass)
{
    setProfile(Profile::loadProfile(name, pass, *settings, parser));
    parser = nullptr; // only apply cmdline proxy settings once
}
/**
 * Changes the loaded profile and notifies listeners.
 * @param p
 */
void Nexus::setProfile(Profile* p)
{
    if (!p) {
        emit profileLoadFailed();
        // Warnings are issued during respective createNew/load calls
        return;
    } else {
        emit profileLoaded();
    }

    emit currentProfileChanged(p);
}

void Nexus::setParser(QCommandLineParser* parser)
{
    this->parser = parser;
}

/**
 * @brief Get desktop GUI widget.
 * @return nullptr if not started, desktop widget otherwise.
 */
Widget* Nexus::getDesktopGUI()
{
    return getInstance().widget;
}

#ifdef Q_OS_MAC
void Nexus::retranslateUi()
{
    viewMenu->menuAction()->setText(tr("View", "OS X Menu bar"));
    windowMenu->menuAction()->setText(tr("Window", "OS X Menu bar"));
    minimizeAction->setText(tr("Minimize", "OS X Menu bar"));
    frontAction->setText((tr("Bring All to Front", "OS X Menu bar")));
}

void Nexus::onWindowStateChanged(Qt::WindowStates state)
{
    minimizeAction->setEnabled(QApplication::activeWindow() != nullptr);

    if (QApplication::activeWindow() != nullptr && sender() == QApplication::activeWindow()) {
        if (state & Qt::WindowFullScreen)
            minimizeAction->setEnabled(false);

        if (state & Qt::WindowFullScreen)
            fullscreenAction->setText(tr("Exit Fullscreen"));
        else
            fullscreenAction->setText(tr("Enter Fullscreen"));

        updateWindows();
    }

    updateWindowsStates();
}

void Nexus::updateWindows()
{
    updateWindowsArg(nullptr);
}

void Nexus::updateWindowsArg(QWindow* closedWindow)
{
    QWindowList windowList = QApplication::topLevelWindows();
    delete windowActions;
    windowActions = new QActionGroup(this);

    windowMenu->addSeparator();

    QAction* dockLast;
    if (!dockMenu->actions().isEmpty())
        dockLast = dockMenu->actions().first();
    else
        dockLast = nullptr;

    QWindow* activeWindow;

    if (QApplication::activeWindow())
        activeWindow = QApplication::activeWindow()->windowHandle();
    else
        activeWindow = nullptr;

    for (int i = 0; i < windowList.size(); ++i) {
        if (closedWindow == windowList[i])
            continue;

        QAction* action = windowActions->addAction(windowList[i]->title());
        action->setCheckable(true);
        action->setChecked(windowList[i] == activeWindow);
        connect(action, &QAction::triggered, [=] { onOpenWindow(windowList[i]); });
        windowMenu->addAction(action);
        dockMenu->insertAction(dockLast, action);
    }

    if (dockLast && !dockLast->isSeparator())
        dockMenu->insertSeparator(dockLast);
}

void Nexus::updateWindowsClosed()
{
    updateWindowsArg(static_cast<QWidget*>(sender())->windowHandle());
}

void Nexus::updateWindowsStates()
{
    bool exists = false;
    QWindowList windowList = QApplication::topLevelWindows();

    for (QWindow* window : windowList) {
        if (!(window->windowState() & Qt::WindowMinimized)) {
            exists = true;
            break;
        }
    }

    frontAction->setEnabled(exists);
}

void Nexus::onOpenWindow(QObject* object)
{
    QWindow* window = static_cast<QWindow*>(object);

    if (window->windowState() & QWindow::Minimized)
        window->showNormal();

    window->raise();
    window->requestActivate();
}

void Nexus::toggleFullscreen()
{
    QWidget* window = QApplication::activeWindow();

    if (window->isFullScreen())
        window->showNormal();
    else
        window->showFullScreen();
}

void Nexus::bringAllToFront()
{
    QWindowList windowList = QApplication::topLevelWindows();
    QWindow* focused = QApplication::focusWindow();

    for (QWindow* window : windowList)
        window->raise();

    focused->raise();
}
#endif
