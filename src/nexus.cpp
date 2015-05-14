#include "nexus.h"
#include "src/core/core.h"
#include "misc/settings.h"
#include "video/camerasource.h"
#include "widget/gui.h"
#include <QThread>
#include <QDebug>
#include <QImageReader>
#include <QFile>

#ifdef Q_OS_ANDROID
#include <src/widget/androidgui.h>
#else
#include <src/widget/widget.h>
#endif

static Nexus* nexus{nullptr};

Nexus::Nexus(QObject *parent) :
    QObject(parent),
    core{nullptr},
    coreThread{nullptr},
    widget{nullptr},
    androidgui{nullptr},
    started{false}
{
}

Nexus::~Nexus()
{
    delete core;
    delete coreThread;
#ifdef Q_OS_ANDROID
    delete androidgui;
#else
    delete widget;
#endif
}

void Nexus::start()
{
    if (started)
        return;

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
    qRegisterMetaType<Core::PasswordType>("Core::PasswordType");
    qRegisterMetaType<std::shared_ptr<VideoFrame>>("std::shared_ptr<VideoFrame>");

    // Create GUI
#ifndef Q_OS_ANDROID
    widget = Widget::getInstance();
#endif

    // Create Core
    QString profilePath = Settings::getInstance().detectProfile();
    coreThread = new QThread(this);
    coreThread->setObjectName("qTox Core");
    core = new Core(coreThread, profilePath);
    core->moveToThread(coreThread);
    connect(coreThread, &QThread::started, core, &Core::start);

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
    connect(core, &Core::blockingClearContacts, widget, &Widget::clearContactsList, Qt::BlockingQueuedConnection);
    connect(core, &Core::friendTypingChanged, widget, &Widget::onFriendTypingChanged);

    connect(core, &Core::messageSentResult, widget, &Widget::onMessageSendResult);
    connect(core, &Core::groupSentResult, widget, &Widget::onGroupSendResult);

    connect(widget, &Widget::statusSet, core, &Core::setStatus);
    connect(widget, &Widget::friendRequested, core, &Core::requestFriendship);
    connect(widget, &Widget::friendRequestAccepted, core, &Core::acceptFriendRequest);
    connect(widget, &Widget::changeProfile, core, &Core::switchConfiguration);
#endif

    // Start Core
    coreThread->start();

    started = true;
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
    return getInstance().core;
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
