/*
    Copyright © 2014-2018 by The qTox Project Contributors

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
#include <QDebug>
#include <QDesktopWidget>
#include <QThread>
#include <cassert>
#include <src/audio/audio.h>

/**
 * @class Nexus
 *
 * This class is in charge of connecting various systems together
 * and forwarding signals appropriately to the right objects,
 * it is in charge of starting the GUI and the Core.
 */

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
 * @brief Hides the main GUI, delete the profile, and shows the login screen
 */
int Nexus::showLogin(const QString& profileName)
{

    // TODO(kriby): Should be replaced by external controller object

    delete widget;
    widget = nullptr;

    delete profile;
    profile = nullptr;

    std::unique_ptr<LoginScreen> loginScreen(new LoginScreen{profileName});
    connectLoginScreen(*loginScreen.get());

    // TODO(kriby): Move core out of profile
    // This is awkward because the core is in the profile
    // The connection order ensures profile will be ready for bootstrap for now
    connect(this, &Nexus::currentProfileChanged, this, &Nexus::bootStrapWithProfile);
    int returnval = loginScreen->exec();
    disconnect(this, &Nexus::currentProfileChanged, this, &Nexus::bootStrapWithProfile);
    return returnval;
}

void Nexus::bootStrapWithProfile(Profile* p)
{
    // kriby: This is a hack until a proper controller is written

    profile = p;

    if (profile) {
        audioControl = std::unique_ptr<IAudioControl>(Audio::makeAudio(*settings));
        assert(audioControl != nullptr);
        profile->getCore()->getAv()->setAudio(*audioControl);
        showMainGUI();
    }
}

void Nexus::setSettings(Settings* settings)
{
    if (this->settings) {
        QObject::disconnect(this, &Nexus::currentProfileChanged, this->settings,
                            &Settings::onCurrentProfileChanged);
        QObject::disconnect(this, &Nexus::saveGlobal, this->settings, &Settings::saveGlobal);
    }
    this->settings = settings;
    if (this->settings) {
        QObject::connect(this, &Nexus::currentProfileChanged, this->settings,
                         &Settings::onCurrentProfileChanged);
        QObject::connect(this, &Nexus::saveGlobal, this->settings, &Settings::saveGlobal);
    }
}

void Nexus::connectLoginScreen(const LoginScreen& loginScreen)
{
    // TODO(kriby): Move connect sequences to a controller class object instead

    // Nexus -> LoginScreen
    QObject::connect(this, &Nexus::profileLoaded, &loginScreen, &LoginScreen::onProfileLoaded);
    // LoginScreen -> Nexus
    QObject::connect(&loginScreen, &LoginScreen::createNewProfile, this, &Nexus::onCreateNewProfile);
    QObject::connect(&loginScreen, &LoginScreen::loadProfile, this, &Nexus::onLoadProfile);
    // LoginScreen -> Settings
    QObject::connect(&loginScreen, &LoginScreen::setAutoLogin, settings, &Settings::onSetAutoLogin);
    // Settings -> LoginScreen
    QObject::connect(settings, &Settings::autoLoginChanged, &loginScreen,
                     &LoginScreen::onAutoLoginChanged);
}

void Nexus::showMainGUI()
{
    // TODO(kriby): Rewrite as view-model connect sequence only, add to a controller class object
    assert(profile);

    // Create GUI
    widget = Widget::getInstance(audioControl.get());

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

    Core* core = profile->getCore();
    connect(core, &Core::requestSent, profile, &Profile::onRequestSent);

    connect(core, &Core::connected, widget, &Widget::onConnected);
    connect(core, &Core::disconnected, widget, &Widget::onDisconnected);
    connect(profile, &Profile::failedToStart, widget, &Widget::onFailedToStartCore,
            Qt::BlockingQueuedConnection);
    connect(profile, &Profile::badProxy, widget, &Widget::onBadProxyCore, Qt::BlockingQueuedConnection);
    connect(core, &Core::statusSet, widget, &Widget::onStatusSet);
    connect(core, &Core::usernameSet, widget, &Widget::setUsername);
    connect(core, &Core::statusMessageSet, widget, &Widget::setStatusMessage);
    connect(core, &Core::friendAdded, widget, &Widget::addFriend);
    connect(core, &Core::failedToAddFriend, widget, &Widget::addFriendFailed);
    connect(core, &Core::friendUsernameChanged, widget, &Widget::onFriendUsernameChanged);
    connect(core, &Core::friendStatusChanged, widget, &Widget::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, widget, &Widget::onFriendStatusMessageChanged);
    connect(core, &Core::friendRequestReceived, widget, &Widget::onFriendRequestReceived);
    connect(core, &Core::friendMessageReceived, widget, &Widget::onFriendMessageReceived);
    connect(core, &Core::groupInviteReceived, widget, &Widget::onGroupInviteReceived);
    connect(core, &Core::groupMessageReceived, widget, &Widget::onGroupMessageReceived);
    connect(core, &Core::groupPeerlistChanged, widget, &Widget::onGroupPeerlistChanged);
    connect(core, &Core::groupPeerNameChanged, widget, &Widget::onGroupPeerNameChanged);
    connect(core, &Core::groupTitleChanged, widget, &Widget::onGroupTitleChanged);
    connect(core, &Core::groupPeerAudioPlaying, widget, &Widget::onGroupPeerAudioPlaying);
    connect(core, &Core::emptyGroupCreated, widget, &Widget::onEmptyGroupCreated);
    connect(core, &Core::groupJoined, widget, &Widget::onGroupJoined);
    connect(core, &Core::friendTypingChanged, widget, &Widget::onFriendTypingChanged);
    connect(core, &Core::groupSentFailed, widget, &Widget::onGroupSendFailed);
    connect(core, &Core::usernameSet, widget, &Widget::refreshPeerListsLocal);

    connect(widget, &Widget::statusSet, core, &Core::setStatus);
    connect(widget, &Widget::friendRequested, core, &Core::requestFriendship);
    connect(widget, &Widget::friendRequestAccepted, core, &Core::acceptFriendRequest);

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

    return nexus.profile->getCore();
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
 * @brief Create a new profile and replace the current one.
 * @param name New username
 * @param pass New password
 */
void Nexus::onCreateNewProfile(QString name, const QString& pass)
{
    Profile* p = Profile::createProfile(name, pass);
    emit profileLoaded(bool(p));

    if (!p) {
        // Warnings are issued during Profile::createProfile
        return;
    }

    emit currentProfileChanged(p);
}

void Nexus::onLoadProfile(QString name, const QString& pass)
{
    Profile* p = Profile::loadProfile(name, pass);
    emit profileLoaded(bool(p));

    if (!p) {
        // Warnings are issued during Profile::loadProfile
        return;
    }
    emit currentProfileChanged(p);
}

/**
 * @brief Get desktop GUI widget.
 * @return nullptr if not started, desktop widget otherwise.
 */
Widget* Nexus::getDesktopGUI()
{
    return getInstance().widget;
}