/*
    Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
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

#include "core.h"
#include "corefile.h"
#include "src/core/coreav.h"
#include "src/core/dhtserver.h"
#include "src/core/icoresettings.h"
#include "src/core/toxlogger.h"
#include "src/core/toxoptions.h"
#include "src/core/toxstring.h"
#include "src/model/groupinvite.h"
#include "src/model/status.h"
#include "src/net/bootstrapnodeupdater.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/util/strongtype.h"

#include <QCoreApplication>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include <QRandomGenerator>
#endif
#include <QRegularExpression>
#include <QString>
#include <QStringBuilder>
#include <QTimer>

#include <cassert>
#include <memory>

const QString Core::TOX_EXT = ".tox";

#define ASSERT_CORE_THREAD assert(QThread::currentThread() == coreThread.get())

namespace {
/**
 * @brief Parse and log toxcore error enums.
 * @param error Error to handle.
 * @return True if no error, false otherwise.
 */

#define PARSE_ERR(err) parseErr(err, __LINE__)

bool parseErr(Tox_Err_Conference_Title error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_TITLE_OK:
        return true;

    case TOX_ERR_CONFERENCE_TITLE_CONFERENCE_NOT_FOUND:
        qCritical() << line << ": Conference title not found";
        return false;

    case TOX_ERR_CONFERENCE_TITLE_INVALID_LENGTH:
        qCritical() << line << ": Invalid conference title length";
        return false;

    case TOX_ERR_CONFERENCE_TITLE_FAIL_SEND:
        qCritical() << line << ": Failed to send title packet";
        return false;

    default:
        qCritical() << line << ": Unknown Tox_Err_Conference_Title error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Friend_Send_Message error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_SEND_MESSAGE_OK:
        return true;

    case TOX_ERR_FRIEND_SEND_MESSAGE_NULL:
        qCritical() << line << "Send friend message passed an unexpected null argument";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND:
        qCritical() << line << "Send friend message could not find friend";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED:
        qCritical() << line << "Send friend message: friend is offline";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_SENDQ:
        qCritical() << line << "Failed to allocate more message queue";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_TOO_LONG:
        qCritical() << line << "Attemped to send message that's too long";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_EMPTY:
        qCritical() << line << "Attempted to send an empty message";
        return false;

    default:
        qCritical() << line << "Unknown friend send message error:" << static_cast<int>(error);
        return false;
    }
}

bool parseErr(Tox_Err_Conference_Send_Message error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_SEND_MESSAGE_OK:
        return true;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_FAIL_SEND:
        qCritical() << line << "Conference message failed to send";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_NO_CONNECTION:
        qCritical() << line << "No connection";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_TOO_LONG:
        qCritical() << line << "Message too long";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Send_Message  error:" << static_cast<int>(error);
        return false;
    }
}

bool parseErr(Tox_Err_Conference_Peer_Query error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_PEER_QUERY_OK:
        return true;

    case TOX_ERR_CONFERENCE_PEER_QUERY_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_PEER_QUERY_NO_CONNECTION:
        qCritical() << line << "No connection";
        return false;

    case TOX_ERR_CONFERENCE_PEER_QUERY_PEER_NOT_FOUND:
        qCritical() << line << "Peer not found";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Peer_Query error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Conference_Join error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_JOIN_OK:
        return true;

    case TOX_ERR_CONFERENCE_JOIN_DUPLICATE:
        qCritical() << line << "Conference duplicate";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_FAIL_SEND:
        qCritical() << line << "Conference join failed to send";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_FRIEND_NOT_FOUND:
        qCritical() << line << "Friend not found";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_INIT_FAIL:
        qCritical() << line << "Init fail";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_INVALID_LENGTH:
        qCritical() << line << "Invalid length";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_WRONG_TYPE:
        qCritical() << line << "Wrong conference type";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Join error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Conference_Get_Type error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_GET_TYPE_OK:
        return true;

    case TOX_ERR_CONFERENCE_GET_TYPE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Get_Type error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Conference_Invite error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_INVITE_OK:
        return true;

    case TOX_ERR_CONFERENCE_INVITE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_INVITE_FAIL_SEND:
        qCritical() << line << "Conference invite failed to send";
        return false;

    case TOX_ERR_CONFERENCE_INVITE_NO_CONNECTION:
        qCritical() << line << "Cannot invite to conference that we're not connected to";
        return false;

    default:
        qWarning() << "Unknown Tox_Err_Conference_Invite error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Conference_New error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_NEW_OK:
        return true;

    case TOX_ERR_CONFERENCE_NEW_INIT:
        qCritical() << line << "The conference instance failed to initialize";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_New error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Friend_By_Public_Key error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_OK:
        return true;

    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_NOT_FOUND:
        // we use this as a check for friendship, so this can be an expected result
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_By_Public_Key error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Bootstrap error, int line)
{
    switch(error) {
    case TOX_ERR_BOOTSTRAP_OK:
        return true;

    case TOX_ERR_BOOTSTRAP_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_BOOTSTRAP_BAD_HOST:
        qCritical() << line << "Could not resolve hostname, or invalid IP address";
        return false;

    case TOX_ERR_BOOTSTRAP_BAD_PORT:
        qCritical() << line << "out of range port";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_bootstrap error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Friend_Add error, int line)
{
    switch(error) {
    case TOX_ERR_FRIEND_ADD_OK:
        return true;

    case TOX_ERR_FRIEND_ADD_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_ADD_TOO_LONG:
        qCritical() << line << "The length of the friend request message exceeded";
        return false;

    case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
        qCritical() << line << "The friend request message was empty.";
        return false;

    case TOX_ERR_FRIEND_ADD_OWN_KEY:
        qCritical() << line << "The friend address belongs to the sending client.";
        return false;

    case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
        qCritical() << line << "The address belongs to a friend that is already on the friend list.";
        return false;

    case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
        qCritical() << line << "The friend address checksum failed.";
        return false;

    case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
        qCritical() << line << "The address belongs to a friend that is already on the friend list.";
        return false;

    case TOX_ERR_FRIEND_ADD_MALLOC:
        qCritical() << line << "A memory allocation failed when trying to increase the friend list size.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Add error code:" << error;
        return false;

    }
}

bool parseErr(Tox_Err_Friend_Delete error, int line)
{
    switch(error) {
    case TOX_ERR_FRIEND_DELETE_OK:
        return true;

    case TOX_ERR_FRIEND_DELETE_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Delete error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Set_Info error, int line)
{
    switch (error) {
    case TOX_ERR_SET_INFO_OK:
        return true;

    case TOX_ERR_SET_INFO_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_SET_INFO_TOO_LONG:
        qCritical() << line << "Information length exceeded maximum permissible size.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Set_Info error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Friend_Query error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_QUERY_OK:
        return true;

    case TOX_ERR_FRIEND_QUERY_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_QUERY_FRIEND_NOT_FOUND:
        qCritical() << line << "The friend_number did not designate a valid friend.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Query error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Friend_Get_Public_Key error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_OK:
        return true;

    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Get_Public_Key error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Friend_Get_Last_Online error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_GET_LAST_ONLINE_OK:
        return true;

    case TOX_ERR_FRIEND_GET_LAST_ONLINE_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Get_Last_Online error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Set_Typing error, int line)
{
    switch (error) {
    case TOX_ERR_SET_TYPING_OK:
        return true;

    case TOX_ERR_SET_TYPING_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Set_Typing error code:" << error;
        return false;
    }
}

bool parseErr(Tox_Err_Conference_Delete error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_DELETE_OK:
        return true;

    case TOX_ERR_CONFERENCE_DELETE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "The conference number passed did not designate a valid conference.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Delete error code:" << error;
        return false;
    }
}

} // namespace

Core::Core(QThread* coreThread)
    : tox(nullptr)
    , toxTimer{new QTimer{this}}
    , coreThread(coreThread)
{
    assert(toxTimer);
    toxTimer->setSingleShot(true);
    connect(toxTimer, &QTimer::timeout, this, &Core::process);
    connect(coreThread, &QThread::finished, toxTimer, &QTimer::stop);
}

Core::~Core()
{
    /*
     * First stop the thread to stop the timer and avoid Core emitting callbacks
     * into an already destructed CoreAV.
     */
    coreThread->exit(0);
    coreThread->wait();

    av.reset();
    tox.reset();
}

/**
 * @brief Registers all toxcore callbacks
 * @param tox Tox instance to register the callbacks on
 */
void Core::registerCallbacks(Tox* tox)
{
    tox_callback_friend_request(tox, onFriendRequest);
    tox_callback_friend_message(tox, onFriendMessage);
    tox_callback_friend_name(tox, onFriendNameChange);
    tox_callback_friend_typing(tox, onFriendTypingChange);
    tox_callback_friend_status_message(tox, onStatusMessageChanged);
    tox_callback_friend_status(tox, onUserStatusChanged);
    tox_callback_friend_connection_status(tox, onConnectionStatusChanged);
    tox_callback_friend_read_receipt(tox, onReadReceiptCallback);
    tox_callback_conference_invite(tox, onGroupInvite);
    tox_callback_conference_message(tox, onGroupMessage);
    tox_callback_conference_peer_list_changed(tox, onGroupPeerListChange);
    tox_callback_conference_peer_name(tox, onGroupPeerNameChange);
    tox_callback_conference_title(tox, onGroupTitleChange);
}

/**
 * @brief Factory method for the Core object
 * @param savedata empty if new profile or saved data else
 * @param settings Settings specific to Core
 * @return nullptr or a Core object ready to start
 */
ToxCorePtr Core::makeToxCore(const QByteArray& savedata, const ICoreSettings* const settings,
                             ToxCoreErrors* err)
{
    QThread* thread = new QThread();
    if (thread == nullptr) {
        qCritical() << "Could not allocate Core thread";
        return {};
    }
    thread->setObjectName("qTox Core");

    auto toxOptions = ToxOptions::makeToxOptions(savedata, settings);
    if (toxOptions == nullptr) {
        qCritical() << "Could not allocate ToxOptions data structure";
        if (err) {
            *err = ToxCoreErrors::ERROR_ALLOC;
        }
        return {};
    }

    ToxCorePtr core(new Core(thread));
    if (core == nullptr) {
        if (err) {
            *err = ToxCoreErrors::ERROR_ALLOC;
        }
        return {};
    }

    Tox_Err_New tox_err;
    core->tox = ToxPtr(tox_new(*toxOptions, &tox_err));

    switch (tox_err) {
    case TOX_ERR_NEW_OK:
        break;

    case TOX_ERR_NEW_LOAD_BAD_FORMAT:
        qCritical() << "Failed to parse Tox save data";
        if (err) {
            *err = ToxCoreErrors::BAD_PROXY;
        }
        return {};

    case TOX_ERR_NEW_PORT_ALLOC:
        if (toxOptions->getIPv6Enabled()) {
            toxOptions->setIPv6Enabled(false);
            core->tox = ToxPtr(tox_new(*toxOptions, &tox_err));
            if (tox_err == TOX_ERR_NEW_OK) {
                qWarning() << "Core failed to start with IPv6, falling back to IPv4. LAN discovery "
                              "may not work properly.";
                break;
            }
        }

        qCritical() << "Can't to bind the port";
        if (err) {
            *err = ToxCoreErrors::FAILED_TO_START;
        }
        return {};

    case TOX_ERR_NEW_PROXY_BAD_HOST:
    case TOX_ERR_NEW_PROXY_BAD_PORT:
    case TOX_ERR_NEW_PROXY_BAD_TYPE:
        qCritical() << "Bad proxy, error code:" << tox_err;
        if (err) {
            *err = ToxCoreErrors::BAD_PROXY;
        }
        return {};

    case TOX_ERR_NEW_PROXY_NOT_FOUND:
        qCritical() << "Proxy not found";
        if (err) {
            *err = ToxCoreErrors::BAD_PROXY;
        }
        return {};

    case TOX_ERR_NEW_LOAD_ENCRYPTED:
        qCritical() << "Attempted to load encrypted Tox save data";
        if (err) {
            *err = ToxCoreErrors::INVALID_SAVE;
        }
        return {};

    case TOX_ERR_NEW_MALLOC:
        qCritical() << "Memory allocation failed";
        if (err) {
            *err = ToxCoreErrors::ERROR_ALLOC;
        }
        return {};

    case TOX_ERR_NEW_NULL:
        qCritical() << "A parameter was null";
        if (err) {
            *err = ToxCoreErrors::FAILED_TO_START;
        }
        return {};

    default:
        qCritical() << "Toxcore failed to start, unknown error code:" << tox_err;
        if (err) {
            *err = ToxCoreErrors::FAILED_TO_START;
        }
        return {};
    }

    // tox should be valid by now
    assert(core->tox != nullptr);

    // toxcore is successfully created, create toxav
    // TODO(sudden6): don't create CoreAv here, Core should be usable without CoreAV
    core->av = CoreAV::makeCoreAV(core->tox.get(), core->coreLoopLock);
    if (!core->av) {
        qCritical() << "Toxav failed to start";
        if (err) {
            *err = ToxCoreErrors::FAILED_TO_START;
        }
        return {};
    }

    // create CoreFile
    core->file = CoreFile::makeCoreFile(core.get(), core->tox.get(), core->coreLoopLock);
    if (!core->file) {
        qCritical() << "CoreFile failed to start";
        if (err) {
            *err = ToxCoreErrors::FAILED_TO_START;
        }
        return {};
    }

    registerCallbacks(core->tox.get());

    // connect the thread with the Core
    connect(thread, &QThread::started, core.get(), &Core::onStarted);
    core->moveToThread(thread);

    // when leaving this function 'core' should be ready for it's start() action or
    // a nullptr
    return core;
}

void Core::onStarted()
{
    ASSERT_CORE_THREAD;

    // One time initialization stuff
    QString name = getUsername();
    if (!name.isEmpty()) {
        emit usernameSet(name);
    }

    QString msg = getStatusMessage();
    if (!msg.isEmpty()) {
        emit statusMessageSet(msg);
    }

    ToxId id = getSelfId();
    // Id comes from toxcore, must be valid
    assert(id.isValid());
    emit idSet(id);

    loadFriends();
    loadGroups();

    process(); // starts its own timer
    av->start();
    emit avReady();
}

/**
 * @brief Starts toxcore and it's event loop, can be called from any thread
 */
void Core::start()
{
    coreThread->start();
}


/**
 * @brief Returns the global widget's Core instance
 */
Core* Core::getInstance()
{
    return Nexus::getCore();
}

const CoreAV* Core::getAv() const
{
    return av.get();
}

CoreAV* Core::getAv()
{
    return av.get();
}

CoreFile* Core::getCoreFile() const
{
    return file.get();
}

/* Using the now commented out statements in checkConnection(), I watched how
 * many ticks disconnects-after-initial-connect lasted. Out of roughly 15 trials,
 * 5 disconnected; 4 were DCd for less than 20 ticks, while the 5th was ~50 ticks.
 * So I set the tolerance here at 25, and initial DCs should be very rare now.
 * This should be able to go to 50 or 100 without affecting legitimate disconnects'
 * downtime, but lets be conservative for now. Edit: now ~~40~~ 30.
 */
#define CORE_DISCONNECT_TOLERANCE 30

/**
 * @brief Processes toxcore events and ensure we stay connected, called by its own timer
 */
void Core::process()
{
    QMutexLocker ml{&coreLoopLock};

    ASSERT_CORE_THREAD;

    static int tolerance = CORE_DISCONNECT_TOLERANCE;
    tox_iterate(tox.get(), this);

#ifdef DEBUG
    // we want to see the debug messages immediately
    fflush(stdout);
#endif

    // TODO(sudden6): recheck if this is still necessary
    if (checkConnection()) {
        tolerance = CORE_DISCONNECT_TOLERANCE;
    } else if (!(--tolerance)) {
        bootstrapDht();
        tolerance = 3 * CORE_DISCONNECT_TOLERANCE;
    }

    unsigned sleeptime =
        qMin(tox_iteration_interval(tox.get()), getCoreFile()->corefileIterationInterval());
    toxTimer->start(sleeptime);
}

bool Core::checkConnection()
{
    ASSERT_CORE_THREAD;
    static bool isConnected = false;
    bool toxConnected = tox_self_get_connection_status(tox.get()) != TOX_CONNECTION_NONE;
    if (toxConnected && !isConnected) {
        qDebug() << "Connected to the DHT";
        emit connected();
    } else if (!toxConnected && isConnected) {
        qDebug() << "Disconnected from the DHT";
        emit disconnected();
    }

    isConnected = toxConnected;
    return toxConnected;
}

/**
 * @brief Connects us to the Tox network
 */
void Core::bootstrapDht()
{
    ASSERT_CORE_THREAD;

    QList<DhtServer> bootstrapNodes = BootstrapNodeUpdater::loadDefaultBootstrapNodes();

    int listSize = bootstrapNodes.size();
    if (!listSize) {
        qWarning() << "No bootstrap node list";
        return;
    }

    int i = 0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    static int j = QRandomGenerator::global()->generate() % listSize;
#else
    static int j = qrand() % listSize;
#endif
    // i think the more we bootstrap, the more we jitter because the more we overwrite nodes
    while (i < 2) {
        const DhtServer& dhtServer = bootstrapNodes[j % listSize];
        QString dhtServerAddress = dhtServer.address.toLatin1();
        QString port = QString::number(dhtServer.port);
        QString name = dhtServer.name;
        qDebug("Connecting to bootstrap node %d", j % listSize);
        QByteArray address = dhtServer.address.toLatin1();
        // TODO: constucting the pk via ToxId is a workaround
        ToxPk pk = ToxId{dhtServer.userId}.getPublicKey();

        const uint8_t* pkPtr = pk.getData();

        Tox_Err_Bootstrap error;
        tox_bootstrap(tox.get(), address.constData(), dhtServer.port, pkPtr, &error);
        PARSE_ERR(error);

        tox_add_tcp_relay(tox.get(), address.constData(), dhtServer.port, pkPtr, &error);
        PARSE_ERR(error);

        ++j;
        ++i;
    }
}

void Core::onFriendRequest(Tox*, const uint8_t* cFriendPk, const uint8_t* cMessage,
                           size_t cMessageSize, void* core)
{
    ToxPk friendPk(cFriendPk);
    QString requestMessage = ToxString(cMessage, cMessageSize).getQString();
    emit static_cast<Core*>(core)->friendRequestReceived(friendPk, requestMessage);
}

void Core::onFriendMessage(Tox*, uint32_t friendId, Tox_Message_Type type, const uint8_t* cMessage,
                           size_t cMessageSize, void* core)
{
    bool isAction = (type == TOX_MESSAGE_TYPE_ACTION);
    QString msg = ToxString(cMessage, cMessageSize).getQString();
    emit static_cast<Core*>(core)->friendMessageReceived(friendId, msg, isAction);
}

void Core::onFriendNameChange(Tox*, uint32_t friendId, const uint8_t* cName, size_t cNameSize, void* core)
{
    QString newName = ToxString(cName, cNameSize).getQString();
    // no saveRequest, this callback is called on every connection, not just on name change
    emit static_cast<Core*>(core)->friendUsernameChanged(friendId, newName);
}

void Core::onFriendTypingChange(Tox*, uint32_t friendId, bool isTyping, void* core)
{
    emit static_cast<Core*>(core)->friendTypingChanged(friendId, isTyping);
}

void Core::onStatusMessageChanged(Tox*, uint32_t friendId, const uint8_t* cMessage,
                                  size_t cMessageSize, void* core)
{
    QString message = ToxString(cMessage, cMessageSize).getQString();
    // no saveRequest, this callback is called on every connection, not just on name change
    emit static_cast<Core*>(core)->friendStatusMessageChanged(friendId, message);
}

void Core::onUserStatusChanged(Tox*, uint32_t friendId, Tox_User_Status userstatus, void* core)
{
    Status::Status status;
    switch (userstatus) {
    case TOX_USER_STATUS_AWAY:
        status = Status::Status::Away;
        break;

    case TOX_USER_STATUS_BUSY:
        status = Status::Status::Busy;
        break;

    default:
        status = Status::Status::Online;
        break;
    }

    // no saveRequest, this callback is called on every connection, not just on name change
    emit static_cast<Core*>(core)->friendStatusChanged(friendId, status);
}

void Core::onConnectionStatusChanged(Tox*, uint32_t friendId, Tox_Connection status, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    Status::Status friendStatus =
        status != TOX_CONNECTION_NONE ? Status::Status::Online : Status::Status::Offline;
    // Ignore Online because it will be emited from onUserStatusChanged
    bool isOffline = friendStatus == Status::Status::Offline;
    if (isOffline) {
        emit core->friendStatusChanged(friendId, friendStatus);
        core->checkLastOnline(friendId);
    }
}

void Core::onGroupInvite(Tox* tox, uint32_t friendId, Tox_Conference_Type type,
                         const uint8_t* cookie, size_t length, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const QByteArray data(reinterpret_cast<const char*>(cookie), length);
    const GroupInvite inviteInfo(friendId, type, data);
    switch (type) {
    case TOX_CONFERENCE_TYPE_TEXT:
        qDebug() << QString("Text group invite by %1").arg(friendId);
        emit core->groupInviteReceived(inviteInfo);
        break;

    case TOX_CONFERENCE_TYPE_AV:
        qDebug() << QString("AV group invite by %1").arg(friendId);
        emit core->groupInviteReceived(inviteInfo);
        break;

    default:
        qWarning() << "Group invite with unknown type " << type;
    }
}

void Core::onGroupMessage(Tox*, uint32_t groupId, uint32_t peerId, Tox_Message_Type type,
                          const uint8_t* cMessage, size_t length, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    bool isAction = type == TOX_MESSAGE_TYPE_ACTION;
    QString message = ToxString(cMessage, length).getQString();
    emit core->groupMessageReceived(groupId, peerId, message, isAction);
}

void Core::onGroupPeerListChange(Tox*, uint32_t groupId, void* vCore)
{
    const auto core = static_cast<Core*>(vCore);
    qDebug() << QString("Group %1 peerlist changed").arg(groupId);
    // no saveRequest, this callback is called on every connection to group peer, not just on brand new peers
    emit core->groupPeerlistChanged(groupId);
}

void Core::onGroupPeerNameChange(Tox*, uint32_t groupId, uint32_t peerId, const uint8_t* name,
                                 size_t length, void* vCore)
{
    const auto newName = ToxString(name, length).getQString();
    qDebug() << QString("Group %1, peer %2, name changed to %3").arg(groupId).arg(peerId).arg(newName);
    auto* core = static_cast<Core*>(vCore);
    auto peerPk = core->getGroupPeerPk(groupId, peerId);
    emit core->groupPeerNameChanged(groupId, peerPk, newName);
}

void Core::onGroupTitleChange(Tox*, uint32_t groupId, uint32_t peerId, const uint8_t* cTitle,
                              size_t length, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    QString author;
    // from tox.h: "If peer_number == UINT32_MAX, then author is unknown (e.g. initial joining the conference)."
    if (peerId != std::numeric_limits<uint32_t>::max()) {
        author = core->getGroupPeerName(groupId, peerId);
    }
    emit core->saveRequest();
    emit core->groupTitleChanged(groupId, author, ToxString(cTitle, length).getQString());
}

void Core::onReadReceiptCallback(Tox*, uint32_t friendId, uint32_t receipt, void* core)
{
    emit static_cast<Core*>(core)->receiptRecieved(friendId, ReceiptNum{receipt});
}

void Core::acceptFriendRequest(const ToxPk& friendPk)
{
    QMutexLocker ml{&coreLoopLock};
    Tox_Err_Friend_Add error;
    uint32_t friendId = tox_friend_add_norequest(tox.get(), friendPk.getData(), &error);
    if (PARSE_ERR(error)) {
        emit saveRequest();
        emit friendAdded(friendId, friendPk);
    } else {
        emit failedToAddFriend(friendPk);
    }
}

/**
 * @brief Checks that sending friendship request is correct and returns error message accordingly
 * @param friendId Id of a friend which request is destined to
 * @param message Friendship request message
 * @return Returns empty string if sending request is correct, according error message otherwise
 */
QString Core::getFriendRequestErrorMessage(const ToxId& friendId, const QString& message) const
{
    QMutexLocker ml{&coreLoopLock};

    if (!friendId.isValid()) {
        return tr("Invalid Tox ID", "Error while sending friend request");
    }

    if (message.isEmpty()) {
        return tr("You need to write a message with your request",
                  "Error while sending friend request");
    }

    if (message.length() > static_cast<int>(tox_max_friend_request_length())) {
        return tr("Your message is too long!", "Error while sending friend request");
    }

    if (hasFriendWithPublicKey(friendId.getPublicKey())) {
        return tr("Friend is already added", "Error while sending friend request");
    }

    return QString{};
}

void Core::requestFriendship(const ToxId& friendId, const QString& message)
{
    QMutexLocker ml{&coreLoopLock};

    ToxPk friendPk = friendId.getPublicKey();
    QString errorMessage = getFriendRequestErrorMessage(friendId, message);
    if (!errorMessage.isNull()) {
        emit failedToAddFriend(friendPk, errorMessage);
        emit saveRequest();
        return;
    }

    ToxString cMessage(message);
    Tox_Err_Friend_Add error;
    uint32_t friendNumber =
        tox_friend_add(tox.get(), friendId.getBytes(), cMessage.data(), cMessage.size(), &error);
    if (PARSE_ERR(error)) {
        qDebug() << "Requested friendship from " << friendNumber;
        emit saveRequest();
        emit friendAdded(friendNumber, friendPk);
        emit requestSent(friendPk, message);
    } else {
        qDebug() << "Failed to send friend request";
        emit failedToAddFriend(friendPk);
    }
}

bool Core::sendMessageWithType(uint32_t friendId, const QString& message, Tox_Message_Type type,
                               ReceiptNum& receipt)
{
    int size = message.toUtf8().size();
    auto maxSize = static_cast<int>(tox_max_message_length());
    if (size > maxSize) {
        qCritical() << "Core::sendMessageWithType called with message of size:" << size
                    << "when max is:" << maxSize << ". Ignoring.";
        return false;
    }

    ToxString cMessage(message);
    Tox_Err_Friend_Send_Message error;
    receipt = ReceiptNum{tox_friend_send_message(tox.get(), friendId, type, cMessage.data(),
                                                 cMessage.size(), &error)};
    if (PARSE_ERR(error)) {
        return true;
    }
    return false;
}

bool Core::sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt)
{
    QMutexLocker ml(&coreLoopLock);
    return sendMessageWithType(friendId, message, TOX_MESSAGE_TYPE_NORMAL, receipt);
}

bool Core::sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt)
{
    QMutexLocker ml(&coreLoopLock);
    return sendMessageWithType(friendId, action, TOX_MESSAGE_TYPE_ACTION, receipt);
}

void Core::sendTyping(uint32_t friendId, bool typing)
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Set_Typing error;
    tox_self_set_typing(tox.get(), friendId, typing, &error);
    if (!PARSE_ERR(error)) {
        emit failedToSetTyping(typing);
    }
}

void Core::sendGroupMessageWithType(int groupId, const QString& message, Tox_Message_Type type)
{
    QMutexLocker ml{&coreLoopLock};

    int size = message.toUtf8().size();
    auto maxSize = static_cast<int>(tox_max_message_length());
    if (size > maxSize) {
        qCritical() << "Core::sendMessageWithType called with message of size:" << size
                    << "when max is:" << maxSize << ". Ignoring.";
        return;
    }

    ToxString cMsg(message);
    Tox_Err_Conference_Send_Message error;
    tox_conference_send_message(tox.get(), groupId, type, cMsg.data(), cMsg.size(), &error);
    if (!PARSE_ERR(error)) {
        emit groupSentFailed(groupId);
        return;
    }
}

void Core::sendGroupMessage(int groupId, const QString& message)
{
    QMutexLocker ml{&coreLoopLock};

    sendGroupMessageWithType(groupId, message, TOX_MESSAGE_TYPE_NORMAL);
}

void Core::sendGroupAction(int groupId, const QString& message)
{
    QMutexLocker ml{&coreLoopLock};

    sendGroupMessageWithType(groupId, message, TOX_MESSAGE_TYPE_ACTION);
}

void Core::changeGroupTitle(int groupId, const QString& title)
{
    QMutexLocker ml{&coreLoopLock};

    ToxString cTitle(title);
    Tox_Err_Conference_Title error;
    tox_conference_set_title(tox.get(), groupId, cTitle.data(), cTitle.size(), &error);
    if (PARSE_ERR(error)) {
        emit saveRequest();
        emit groupTitleChanged(groupId, getUsername(), title);
    }
}

void Core::removeFriend(uint32_t friendId)
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Friend_Delete error;
    tox_friend_delete(tox.get(), friendId, &error);
    if (!PARSE_ERR(error)) {
        emit failedToRemoveFriend(friendId);
        return;
    }

    emit saveRequest();
    emit friendRemoved(friendId);
}

void Core::removeGroup(int groupId)
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Conference_Delete error;
    tox_conference_delete(tox.get(), groupId, &error);
    if (PARSE_ERR(error)) {
        emit saveRequest();
        av->leaveGroupCall(groupId);
    }
}

/**
 * @brief Returns our username, or an empty string on failure
 */
QString Core::getUsername() const
{
    QMutexLocker ml{&coreLoopLock};

    QString sname;
    if (!tox) {
        return sname;
    }

    int size = tox_self_get_name_size(tox.get());
    if (!size) {
        return {};
    }
    std::vector<uint8_t> nameBuf(size);
    tox_self_get_name(tox.get(), nameBuf.data());
    return ToxString(nameBuf.data(), size).getQString();
}

void Core::setUsername(const QString& username)
{
    QMutexLocker ml{&coreLoopLock};

    if (username == getUsername()) {
        return;
    }

    ToxString cUsername(username);
    Tox_Err_Set_Info error;
    tox_self_set_name(tox.get(), cUsername.data(), cUsername.size(), &error);
    if (!PARSE_ERR(error)) {
        emit failedToSetUsername(username);
        return;
    }

    emit usernameSet(username);
    emit saveRequest();
}

/**
 * @brief Returns our Tox ID
 */
ToxId Core::getSelfId() const
{
    QMutexLocker ml{&coreLoopLock};

    uint8_t friendId[TOX_ADDRESS_SIZE] = {0x00};
    tox_self_get_address(tox.get(), friendId);
    return ToxId(friendId, TOX_ADDRESS_SIZE);
}

/**
 * @brief Gets self public key
 * @return Self PK
 */
ToxPk Core::getSelfPublicKey() const
{
    QMutexLocker ml{&coreLoopLock};

    uint8_t friendId[TOX_ADDRESS_SIZE] = {0x00};
    tox_self_get_address(tox.get(), friendId);
    return ToxPk(friendId);
}

/**
 * @brief Returns our public and private keys
 */
QPair<QByteArray, QByteArray> Core::getKeypair() const
{
    QMutexLocker ml{&coreLoopLock};

    QPair<QByteArray, QByteArray> keypair;
    assert(tox != nullptr);

    QByteArray pk(TOX_PUBLIC_KEY_SIZE, 0x00);
    QByteArray sk(TOX_SECRET_KEY_SIZE, 0x00);
    tox_self_get_public_key(tox.get(), reinterpret_cast<uint8_t*>(pk.data()));
    tox_self_get_secret_key(tox.get(), reinterpret_cast<uint8_t*>(sk.data()));
    keypair.first = pk;
    keypair.second = sk;
    return keypair;
}

/**
 * @brief Returns our status message, or an empty string on failure
 */
QString Core::getStatusMessage() const
{
    QMutexLocker ml{&coreLoopLock};

    assert(tox != nullptr);

    size_t size = tox_self_get_status_message_size(tox.get());
    if (!size) {
        return {};
    }
    std::vector<uint8_t> nameBuf(size);
    tox_self_get_status_message(tox.get(), nameBuf.data());
    return ToxString(nameBuf.data(), size).getQString();
}

/**
 * @brief Returns our user status
 */
Status::Status Core::getStatus() const
{
    QMutexLocker ml{&coreLoopLock};

    return static_cast<Status::Status>(tox_self_get_status(tox.get()));
}

void Core::setStatusMessage(const QString& message)
{
    QMutexLocker ml{&coreLoopLock};

    if (message == getStatusMessage()) {
        return;
    }

    ToxString cMessage(message);
    Tox_Err_Set_Info error;
    tox_self_set_status_message(tox.get(), cMessage.data(), cMessage.size(), &error);
    if (!PARSE_ERR(error)) {
        emit failedToSetStatusMessage(message);
        return;
    }

    emit saveRequest();
    emit statusMessageSet(message);
}

void Core::setStatus(Status::Status status)
{
    QMutexLocker ml{&coreLoopLock};

    Tox_User_Status userstatus;
    switch (status) {
    case Status::Status::Online:
        userstatus = TOX_USER_STATUS_NONE;
        break;

    case Status::Status::Away:
        userstatus = TOX_USER_STATUS_AWAY;
        break;

    case Status::Status::Busy:
        userstatus = TOX_USER_STATUS_BUSY;
        break;

    default:
        return;
        break;
    }

    tox_self_set_status(tox.get(), userstatus);
    emit saveRequest();
    emit statusSet(status);
}

/**
 * @brief Returns the unencrypted tox save data
 */
QByteArray Core::getToxSaveData()
{
    QMutexLocker ml{&coreLoopLock};

    uint32_t fileSize = tox_get_savedata_size(tox.get());
    QByteArray data;
    data.resize(fileSize);
    tox_get_savedata(tox.get(), reinterpret_cast<uint8_t*>(data.data()));
    return data;
}

void Core::loadFriends()
{
    QMutexLocker ml{&coreLoopLock};

    const size_t friendCount = tox_self_get_friend_list_size(tox.get());
    if (friendCount == 0) {
        return;
    }

    std::vector<uint32_t> ids(friendCount);
    tox_self_get_friend_list(tox.get(), ids.data());
    uint8_t friendPk[TOX_PUBLIC_KEY_SIZE] = {0x00};
    for (size_t i = 0; i < friendCount; ++i) {
        Tox_Err_Friend_Get_Public_Key keyError;
        tox_friend_get_public_key(tox.get(), ids[i], friendPk, &keyError);
        if (!PARSE_ERR(keyError)) {
            continue;
        }
        emit friendAdded(ids[i], ToxPk(friendPk));
        emit friendUsernameChanged(ids[i], getFriendUsername(ids[i]));
        Tox_Err_Friend_Query queryError;
        size_t statusMessageSize = tox_friend_get_status_message_size(tox.get(), ids[i], &queryError);
        if (PARSE_ERR(queryError) && statusMessageSize) {
            std::vector<uint8_t> messageData(statusMessageSize);
            tox_friend_get_status_message(tox.get(), ids[i], messageData.data(), &queryError);
            QString friendStatusMessage = ToxString(messageData.data(), statusMessageSize).getQString();
            emit friendStatusMessageChanged(ids[i], friendStatusMessage);
        }
        checkLastOnline(ids[i]);
    }
}

void Core::loadGroups()
{
    QMutexLocker ml{&coreLoopLock};

    const size_t groupCount = tox_conference_get_chatlist_size(tox.get());
    if (groupCount == 0) {
        return;
    }

    std::vector<uint32_t> groupNumbers(groupCount);
    tox_conference_get_chatlist(tox.get(), groupNumbers.data());

    for (size_t i = 0; i < groupCount; ++i) {
        Tox_Err_Conference_Title error;
        QString name;
        const auto groupNumber = groupNumbers[i];
        size_t titleSize = tox_conference_get_title_size(tox.get(), groupNumber, &error);
        const GroupId persistentId = getGroupPersistentId(groupNumber);
        const QString defaultName = tr("Groupchat %1").arg(persistentId.toString().left(8));
        if (PARSE_ERR(error) || !titleSize) {
            std::vector<uint8_t> nameBuf(titleSize);
            tox_conference_get_title(tox.get(), groupNumber, nameBuf.data(), &error);
            if (PARSE_ERR(error)) {
                name = ToxString(nameBuf.data(), titleSize).getQString();
            } else {
                name = defaultName;
            }
        } else {
            name = defaultName;
        }
        if (getGroupAvEnabled(groupNumber)) {
            if (toxav_groupchat_enable_av(tox.get(), groupNumber, CoreAV::groupCallCallback, this)) {
                qCritical() << "Failed to enable audio on loaded group" << groupNumber;
            }
        }
        emit emptyGroupCreated(groupNumber, persistentId, name);
    }
}

void Core::checkLastOnline(uint32_t friendId)
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Friend_Get_Last_Online error;
    const uint64_t lastOnline = tox_friend_get_last_online(tox.get(), friendId, &error);
    if (PARSE_ERR(error)) {
        emit friendLastSeenChanged(friendId, QDateTime::fromTime_t(lastOnline));
    }
}

/**
 * @brief Returns the list of friendIds in our friendlist, an empty list on error
 */
QVector<uint32_t> Core::getFriendList() const
{
    QMutexLocker ml{&coreLoopLock};

    QVector<uint32_t> friends;
    friends.resize(tox_self_get_friend_list_size(tox.get()));
    tox_self_get_friend_list(tox.get(), friends.data());
    return friends;
}

GroupId Core::getGroupPersistentId(uint32_t groupNumber) const
{
    QMutexLocker ml{&coreLoopLock};

    std::vector<uint8_t> idBuff(TOX_CONFERENCE_UID_SIZE);
    if (tox_conference_get_id(tox.get(), groupNumber,
                              idBuff.data())) {
        return GroupId{idBuff.data()};
    } else {
        qCritical() << "Failed to get conference ID of group" << groupNumber;
        return {};
    }
}

/**
 * @brief Get number of peers in the conference.
 * @return The number of peers in the conference. UINT32_MAX on failure.
 */
uint32_t Core::getGroupNumberPeers(int groupId) const
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Conference_Peer_Query error;
    uint32_t count = tox_conference_peer_count(tox.get(), groupId, &error);
    if (!PARSE_ERR(error)) {
        return std::numeric_limits<uint32_t>::max();
    }

    return count;
}

/**
 * @brief Get the name of a peer of a group
 */
QString Core::getGroupPeerName(int groupId, int peerId) const
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Conference_Peer_Query error;
    size_t length = tox_conference_peer_get_name_size(tox.get(), groupId, peerId, &error);
    if (!PARSE_ERR(error) || !length) {
        return QString{};
    }

    std::vector<uint8_t> nameBuf(length);
    tox_conference_peer_get_name(tox.get(), groupId, peerId, nameBuf.data(), &error);
    if (!PARSE_ERR(error)) {
        return QString{};
    }

    return ToxString(nameBuf.data(), length).getQString();
}

/**
 * @brief Get the public key of a peer of a group
 */
ToxPk Core::getGroupPeerPk(int groupId, int peerId) const
{
    QMutexLocker ml{&coreLoopLock};

    uint8_t friendPk[TOX_PUBLIC_KEY_SIZE] = {0x00};
    Tox_Err_Conference_Peer_Query error;
    tox_conference_peer_get_public_key(tox.get(), groupId, peerId, friendPk, &error);
    if (!PARSE_ERR(error)) {
        return ToxPk{};
    }

    return ToxPk(friendPk);
}

/**
 * @brief Get the names of the peers of a group
 */
QStringList Core::getGroupPeerNames(int groupId) const
{
    QMutexLocker ml{&coreLoopLock};

    assert(tox != nullptr);

    uint32_t nPeers = getGroupNumberPeers(groupId);
    if (nPeers == std::numeric_limits<uint32_t>::max()) {
        qWarning() << "getGroupPeerNames: Unable to get number of peers";
        return {};
    }

    QStringList names;
    for (int i = 0; i < static_cast<int>(nPeers); ++i) {
        Tox_Err_Conference_Peer_Query error;
        size_t length = tox_conference_peer_get_name_size(tox.get(), groupId, i, &error);

        if (!PARSE_ERR(error) || !length) {
            names.append(QString());
            continue;
        }

        std::vector<uint8_t> nameBuf(length);
        tox_conference_peer_get_name(tox.get(), groupId, i, nameBuf.data(), &error);
        if (PARSE_ERR(error)) {
            names.append(ToxString(nameBuf.data(), length).getQString());
        } else {
            names.append(QString());
        }
    }

    assert(names.size() == static_cast<int>(nPeers));

    return names;
}

/**
 * @brief Check, that group has audio or video stream
 * @param groupId Id of group to check
 * @return True for AV groups, false for text-only groups
 */
bool Core::getGroupAvEnabled(int groupId) const
{
    QMutexLocker ml{&coreLoopLock};
    Tox_Err_Conference_Get_Type error;
    Tox_Conference_Type type = tox_conference_get_type(tox.get(), groupId, &error);
    PARSE_ERR(error);
    // would be nice to indicate to caller that we don't actually know..
    return type == TOX_CONFERENCE_TYPE_AV;
}

/**
 * @brief Accept a groupchat invite.
 * @param inviteInfo Object which contains info about group invitation
 *
 * @return Conference number on success, UINT32_MAX on failure.
 */
uint32_t Core::joinGroupchat(const GroupInvite& inviteInfo)
{
    QMutexLocker ml{&coreLoopLock};

    const uint32_t friendId = inviteInfo.getFriendId();
    const uint8_t confType = inviteInfo.getType();
    const QByteArray invite = inviteInfo.getInvite();
    const uint8_t* const cookie = reinterpret_cast<const uint8_t*>(invite.data());
    const size_t cookieLength = invite.length();
    uint32_t groupNum{std::numeric_limits<uint32_t>::max()};
    switch (confType) {
    case TOX_CONFERENCE_TYPE_TEXT: {
        qDebug() << QString("Trying to accept invite for text group chat sent by friend %1").arg(friendId);
        Tox_Err_Conference_Join error;
        groupNum = tox_conference_join(tox.get(), friendId, cookie, cookieLength, &error);
        if (!PARSE_ERR(error)) {
            groupNum = std::numeric_limits<uint32_t>::max();
        }
        break;
    }
    case TOX_CONFERENCE_TYPE_AV: {
        qDebug() << QString("Trying to join AV groupchat invite sent by friend %1").arg(friendId);
        groupNum = toxav_join_av_groupchat(tox.get(), friendId, cookie, cookieLength,
                                           CoreAV::groupCallCallback, const_cast<Core*>(this));
        break;
    }
    default:
        qWarning() << "joinGroupchat: Unknown groupchat type " << confType;
    }
    if (groupNum != std::numeric_limits<uint32_t>::max()) {
        emit saveRequest();
        emit groupJoined(groupNum, getGroupPersistentId(groupNum));
    }
    return groupNum;
}

void Core::groupInviteFriend(uint32_t friendId, int groupId)
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Conference_Invite error;
    tox_conference_invite(tox.get(), friendId, groupId, &error);
    PARSE_ERR(error);
}

int Core::createGroup(uint8_t type)
{
    QMutexLocker ml{&coreLoopLock};

    if (type == TOX_CONFERENCE_TYPE_TEXT) {
        Tox_Err_Conference_New error;
        uint32_t groupId = tox_conference_new(tox.get(), &error);
        if (PARSE_ERR(error)) {
            emit saveRequest();
            emit emptyGroupCreated(groupId, getGroupPersistentId(groupId));
            return groupId;
        } else {
            return std::numeric_limits<uint32_t>::max();
        }
    } else if (type == TOX_CONFERENCE_TYPE_AV) {
        // unlike tox_conference_new, toxav_add_av_groupchat does not have an error enum, so -1 group number is our
        // only indication of an error
        int groupId = toxav_add_av_groupchat(tox.get(), CoreAV::groupCallCallback, this);
        if (groupId != -1) {
            emit saveRequest();
            emit emptyGroupCreated(groupId, getGroupPersistentId(groupId));
        } else {
            qCritical() << "Unknown error creating AV groupchat";
        }
        return groupId;
    } else {
        qWarning() << "createGroup: Unknown type " << type;
        return -1;
    }
}

/**
 * @brief Checks if a friend is online. Unknown friends are considered offline.
 */
bool Core::isFriendOnline(uint32_t friendId) const
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Friend_Query error;
    Tox_Connection connection = tox_friend_get_connection_status(tox.get(), friendId, &error);
    PARSE_ERR(error);
    return connection != TOX_CONNECTION_NONE;
}

/**
 * @brief Checks if we have a friend by public key
 */
bool Core::hasFriendWithPublicKey(const ToxPk& publicKey) const
{
    QMutexLocker ml{&coreLoopLock};

    if (publicKey.isEmpty()) {
        return false;
    }

    Tox_Err_Friend_By_Public_Key error;
    (void)tox_friend_by_public_key(tox.get(), publicKey.getData(), &error);
    return PARSE_ERR(error);
}

/**
 * @brief Get the public key part of the ToxID only
 */
ToxPk Core::getFriendPublicKey(uint32_t friendNumber) const
{
    QMutexLocker ml{&coreLoopLock};

    uint8_t rawid[TOX_PUBLIC_KEY_SIZE];
    Tox_Err_Friend_Get_Public_Key error;
    tox_friend_get_public_key(tox.get(), friendNumber, rawid, &error);
    if (!PARSE_ERR(error)) {
        qWarning() << "getFriendPublicKey: Getting public key failed";
        return ToxPk();
    }

    return ToxPk(rawid);
}

/**
 * @brief Get the username of a friend
 */
QString Core::getFriendUsername(uint32_t friendnumber) const
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Friend_Query error;
    size_t nameSize = tox_friend_get_name_size(tox.get(), friendnumber, &error);
    if (!PARSE_ERR(error) || !nameSize) {
        return QString();
    }

    std::vector<uint8_t> nameBuf(nameSize);
    tox_friend_get_name(tox.get(), friendnumber, nameBuf.data(), &error);
    if (!PARSE_ERR(error)) {
        return QString();
    }
    return ToxString(nameBuf.data(), nameSize).getQString();
}

QStringList Core::splitMessage(const QString& message)
{
    QStringList splittedMsgs;
    QByteArray ba_message{message.toUtf8()};

    /*
     * TODO: Remove this hack; the reported max message length we receive from c-toxcore
     * as of 08-02-2019 is inaccurate, causing us to generate too large messages when splitting
     * them up.
     *
     * The inconsistency lies in c-toxcore group.c:2480 using MAX_GROUP_MESSAGE_DATA_LEN to verify
     * message size is within limit, but tox_max_message_length giving a different size limit to us.
     *
     * (uint32_t tox_max_message_length(void); declared in tox.h, unable to see explicit definition)
     */
    const auto maxLen = static_cast<int>(tox_max_message_length()) - 50;

    while (ba_message.size() > maxLen) {
        int splitPos = ba_message.lastIndexOf('\n', maxLen - 1);

        if (splitPos <= 0) {
            splitPos = ba_message.lastIndexOf(' ', maxLen - 1);
        }

        if (splitPos <= 0) {
            constexpr uint8_t firstOfMultiByteMask = 0xC0;
            constexpr uint8_t multiByteMask = 0x80;
            splitPos = maxLen;
            // don't split a utf8 character
            if ((ba_message[splitPos] & multiByteMask) == multiByteMask) {
                while ((ba_message[splitPos] & firstOfMultiByteMask) != firstOfMultiByteMask) {
                    --splitPos;
                }
            }
            --splitPos;
        }
        splittedMsgs.append(QString{ba_message.left(splitPos + 1)});
        ba_message = ba_message.mid(splitPos + 1);
    }

    splittedMsgs.append(QString{ba_message});
    return splittedMsgs;
}

QString Core::getPeerName(const ToxPk& id) const
{
    QMutexLocker ml{&coreLoopLock};

    Tox_Err_Friend_By_Public_Key keyError;
    uint32_t friendId = tox_friend_by_public_key(tox.get(), id.getData(), &keyError);
    if (!PARSE_ERR(keyError)) {
        qWarning() << "getPeerName: No such peer";
        return {};
    }

    Tox_Err_Friend_Query queryError;
    const size_t nameSize = tox_friend_get_name_size(tox.get(), friendId, &queryError);
    if (!PARSE_ERR(queryError) || !nameSize) {
        return {};
    }

    std::vector<uint8_t> nameBuf(nameSize);
    tox_friend_get_name(tox.get(), friendId, nameBuf.data(), &queryError);
    if (!PARSE_ERR(queryError)) {
        qWarning() << "getPeerName: Can't get name of friend " + QString().setNum(friendId);
        return {};
    }

    return ToxString(nameBuf.data(), nameSize).getQString();
}

/**
 * @brief Sets the NoSpam value to prevent friend request spam
 * @param nospam an arbitrary which becomes part of the Tox ID
 */
void Core::setNospam(uint32_t nospam)
{
    QMutexLocker ml{&coreLoopLock};

    tox_self_set_nospam(tox.get(), nospam);
    emit idSet(getSelfId());
}
