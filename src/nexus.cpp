#include "nexus.h"
#include "profile.h"
#include "src/core/core.h"
#include "misc/settings.h"
#include "video/camerasource.h"
#include "widget/gui.h"
#include "widget/loginscreen.h"
#include <QThread>
#include <QDebug>
#include <QImageReader>
#include <QFile>
#include <QApplication>
#include <cassert>

#ifdef Q_OS_ANDROID
#include <src/widget/androidgui.h>
#else
#include <src/widget/widget.h>
#include <QDesktopWidget>
#endif

static Nexus* nexus{nullptr};

Nexus::Nexus(QObject *parent) :
    QObject(parent),
    profile{nullptr},
    widget{nullptr},
    androidgui{nullptr},
    loginScreen{nullptr}
{
}

Nexus::~Nexus()
{
#ifdef Q_OS_ANDROID
    delete androidgui;
#else
    delete widget;
#endif
    delete loginScreen;
    delete profile;
    if (profile)
        Settings::getInstance().save();
}

void Nexus::start()
{
    qDebug() << "Starting up";

    // Setup the environment
    qRegisterMetaType<Status>("Status");
    qRegisterMetaType<vpx_image>("vpx_image");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<const int16_t*>("const int16_t*");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<QPixmap>("QPixmap");
    qRegisterMetaType<ToxFile>("ToxFile");
    qRegisterMetaType<ToxFile::FileDirection>("ToxFile::FileDirection");
    qRegisterMetaType<std::shared_ptr<VideoFrame>>("std::shared_ptr<VideoFrame>");

    loginScreen = new LoginScreen();

    if (profile)
        showMainGUI();
    else
        showLogin();
}

void Nexus::showLogin()
{
#ifdef Q_OS_ANDROID
    delete androidui;
    androidgui = nullptr;
#else
    delete widget;
    widget = nullptr;
#endif

    delete profile;
    profile = nullptr;

    loginScreen->reset();
#ifndef Q_OS_ANDROID
    loginScreen->move(QApplication::desktop()->screen()->rect().center() - loginScreen->rect().center());
#endif
    loginScreen->show();
    ((QApplication*)qApp)->setQuitOnLastWindowClosed(true);
}

void Nexus::showMainGUI()
{
    assert(profile);

    ((QApplication*)qApp)->setQuitOnLastWindowClosed(false);
    loginScreen->close();

    // Create GUI
#ifndef Q_OS_ANDROID
    widget = Widget::getInstance();
#endif

    // Start GUI
#ifdef Q_OS_ANDROID
    androidgui = new AndroidGUI;
    androidgui->show();
#else
    widget->init();
#endif
    GUI::getInstance();

    // Zetok protection
    // There are small instants on startup during which no
    // profile is loaded but the GUI could still receive events,
    // e.g. between two modal windows. Disable the GUI to prevent that.
    GUI::setEnabled(false);

    // Connections
    Core* core = profile->getCore();
#ifdef Q_OS_ANDROID
    connect(core, &Core::connected, androidgui, &AndroidGUI::onConnected);
    connect(core, &Core::disconnected, androidgui, &AndroidGUI::onDisconnected);
    //connect(core, &Core::failedToStart, androidgui, &AndroidGUI::onFailedToStartCore, Qt::BlockingQueuedConnection);
    //connect(core, &Core::badProxy, androidgui, &AndroidGUI::onBadProxyCore, Qt::BlockingQueuedConnection);
    connect(core, &Core::statusSet, androidgui, &AndroidGUI::onStatusSet);
    connect(core, &Core::usernameSet, androidgui, &AndroidGUI::setUsername);
    connect(core, &Core::statusMessageSet, androidgui, &AndroidGUI::setStatusMessage);
    connect(core, &Core::selfAvatarChanged, androidgui, &AndroidGUI::onSelfAvatarLoaded);

    connect(androidgui, &AndroidGUI::statusSet, core, &Core::setStatus);
    //connect(androidgui, &AndroidGUI::friendRequested, core, &Core::requestFriendship);
    //connect(androidgui, &AndroidGUI::friendRequestAccepted, core, &Core::acceptFriendRequest);
    //connect(androidgui, &AndroidGUI::changeProfile, core, &Core::switchConfiguration);
#else
    connect(core, &Core::connected,                  widget, &Widget::onConnected);
    connect(core, &Core::disconnected,               widget, &Widget::onDisconnected);
    connect(core, &Core::failedToStart,              widget, &Widget::onFailedToStartCore, Qt::BlockingQueuedConnection);
    connect(core, &Core::badProxy,                   widget, &Widget::onBadProxyCore, Qt::BlockingQueuedConnection);
    connect(core, &Core::statusSet,                  widget, &Widget::onStatusSet);
    connect(core, &Core::usernameSet,                widget, &Widget::setUsername);
    connect(core, &Core::statusMessageSet,           widget, &Widget::setStatusMessage);
    connect(core, &Core::selfAvatarChanged,          widget, &Widget::onSelfAvatarLoaded);
    connect(core, &Core::friendAdded,                widget, &Widget::addFriend);
    connect(core, &Core::failedToAddFriend,          widget, &Widget::addFriendFailed);
    connect(core, &Core::friendUsernameChanged,      widget, &Widget::onFriendUsernameChanged);
    connect(core, &Core::friendStatusChanged,        widget, &Widget::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, widget, &Widget::onFriendStatusMessageChanged);
    connect(core, &Core::friendRequestReceived,      widget, &Widget::onFriendRequestReceived);
    connect(core, &Core::friendMessageReceived,      widget, &Widget::onFriendMessageReceived);
    connect(core, &Core::receiptRecieved,            widget, &Widget::onReceiptRecieved);
    connect(core, &Core::groupInviteReceived,        widget, &Widget::onGroupInviteReceived);
    connect(core, &Core::groupMessageReceived,       widget, &Widget::onGroupMessageReceived);
    connect(core, &Core::groupNamelistChanged,       widget, &Widget::onGroupNamelistChanged);
    connect(core, &Core::groupTitleChanged,          widget, &Widget::onGroupTitleChanged);
    connect(core, &Core::groupPeerAudioPlaying,      widget, &Widget::onGroupPeerAudioPlaying);
    connect(core, &Core::emptyGroupCreated, widget, &Widget::onEmptyGroupCreated);
    connect(core, &Core::avInvite, widget, &Widget::playRingtone);
    connect(core, &Core::friendTypingChanged, widget, &Widget::onFriendTypingChanged);

    connect(core, &Core::messageSentResult, widget, &Widget::onMessageSendResult);
    connect(core, &Core::groupSentResult, widget, &Widget::onGroupSendResult);

    connect(widget, &Widget::statusSet, core, &Core::setStatus);
    connect(widget, &Widget::friendRequested, core, &Core::requestFriendship);
    connect(widget, &Widget::friendRequestAccepted, core, &Core::acceptFriendRequest);
#endif

    profile->startCore();
}

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

Core* Nexus::getCore()
{
    Nexus& nexus = getInstance();
    if (!nexus.profile)
        return nullptr;
    return nexus.profile->getCore();
}

Profile* Nexus::getProfile()
{
    return getInstance().profile;
}

void Nexus::setProfile(Profile* profile)
{
    getInstance().profile = profile;
}

AndroidGUI* Nexus::getAndroidGUI()
{
    return getInstance().androidgui;
}

Widget* Nexus::getDesktopGUI()
{
    return getInstance().widget;
}

QString Nexus::getSupportedImageFilter()
{
  QString res;
  for (auto type : QImageReader::supportedImageFormats())
    res += QString("*.%1 ").arg(QString(type));

  return tr("Images (%1)", "filetype filter").arg(res.left(res.size()-1));
}

bool Nexus::tryRemoveFile(const QString& filepath)
{
    QFile tmp(filepath);
    bool writable = tmp.open(QIODevice::WriteOnly);
    tmp.remove();
    return writable;
}
