#include "nexus.h"
#include "core.h"
#include "misc/settings.h"
#include "video/camera.h"
#include "widget/gui.h"
#include <QThread>
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <src/widget/androidgui.h>
#else
#include <src/widget/widget.h>
#endif

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
    qDebug() << "Nexus: Starting up";

    // Setup the environment
    qRegisterMetaType<Status>("Status");
    qRegisterMetaType<vpx_image>("vpx_image");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<const int16_t*>("const int16_t*");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<QPixmap>("QPixmap");
    qRegisterMetaType<ToxFile>("ToxFile");
    qRegisterMetaType<ToxFile::FileDirection>("ToxFile::FileDirection");
    qRegisterMetaType<Core::PasswordType>("Core::PasswordType");

    // Create Core
    QString profilePath = Settings::getInstance().detectProfile();
    coreThread = new QThread(this);
    coreThread->setObjectName("qTox Core");
    core = new Core(Camera::getInstance(), coreThread, profilePath);
    core->moveToThread(coreThread);
    connect(coreThread, &QThread::started, core, &Core::start);

    // Start GUI
#ifdef Q_OS_ANDROID
    androidgui = new AndroidGUI;
    androidgui->show();
#else
    widget = Widget::getInstance();
#endif
    GUI::getInstance();

    // Connections
#ifndef Q_OS_ANDROID
    connect(core, &Core::connected, widget, &Widget::onConnected);
    connect(core, &Core::disconnected, widget, &Widget::onDisconnected);
    connect(core, &Core::failedToStart, widget, &Widget::onFailedToStartCore);
    connect(core, &Core::badProxy, widget, &Widget::onBadProxyCore);
    connect(core, &Core::statusSet, widget, &Widget::onStatusSet);
    connect(core, &Core::usernameSet, widget, &Widget::setUsername);
    connect(core, &Core::statusMessageSet, widget, &Widget::setStatusMessage);
    connect(core, &Core::selfAvatarChanged, widget, &Widget::onSelfAvatarLoaded);
    connect(core, &Core::friendAdded, widget, &Widget::addFriend);
    connect(core, &Core::failedToAddFriend, widget, &Widget::addFriendFailed);
    connect(core, &Core::friendUsernameChanged, widget, &Widget::onFriendUsernameChanged);
    connect(core, &Core::friendStatusChanged, widget, &Widget::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, widget, &Widget::onFriendStatusMessageChanged);
    connect(core, &Core::friendRequestReceived, widget, &Widget::onFriendRequestReceived);
    connect(core, &Core::friendMessageReceived, widget, &Widget::onFriendMessageReceived);
    connect(core, &Core::receiptRecieved, widget, &Widget::onReceiptRecieved);
    connect(core, &Core::groupInviteReceived, widget, &Widget::onGroupInviteReceived);
    connect(core, &Core::groupMessageReceived, widget, &Widget::onGroupMessageReceived);
    connect(core, &Core::groupNamelistChanged, widget, &Widget::onGroupNamelistChanged);
    connect(core, &Core::groupTitleChanged, widget, &Widget::onGroupTitleChanged);
    connect(core, &Core::emptyGroupCreated, widget, &Widget::onEmptyGroupCreated);
    connect(core, &Core::avInvite, widget, &Widget::playRingtone);
    connect(core, &Core::blockingClearContacts, widget, &Widget::clearContactsList, Qt::BlockingQueuedConnection);
    connect(core, &Core::friendTypingChanged, widget, &Widget::onFriendTypingChanged);

    connect(core, SIGNAL(messageSentResult(int,QString,int)), widget, SLOT(onMessageSendResult(int,QString,int)));
    connect(core, SIGNAL(groupSentResult(int,QString,int)), widget, SLOT(onGroupSendResult(int,QString,int)));

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
    static Nexus nexus;
    return nexus;
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
