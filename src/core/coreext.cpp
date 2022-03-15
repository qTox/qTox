/*
    Copyright © 2019-2020 by The qTox Project Contributors

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

#include "coreext.h"
#include "toxstring.h"

#include <QDateTime>
#include <QTimeZone>
#include <QtCore>

#include <memory>
#include <cassert>

extern "C" {
#include <toxext/toxext.h>
#include <tox_extension_messages.h>
}

std::unique_ptr<CoreExt> CoreExt::makeCoreExt(Tox* core) {
    auto toxExtPtr = toxext_init(core);
    if (!toxExtPtr) {
        return nullptr;
    }

    auto toxExt = ExtensionPtr<ToxExt>(toxExtPtr, toxext_free);
    return std::unique_ptr<CoreExt>(new CoreExt(std::move(toxExt)));
}

CoreExt::CoreExt(ExtensionPtr<ToxExt> toxExt_)
    : toxExt(std::move(toxExt_))
    , toxExtMessages(nullptr, nullptr)
{
    toxExtMessages = ExtensionPtr<ToxExtensionMessages>(
        tox_extension_messages_register(
            toxExt.get(),
            CoreExt::onExtendedMessageReceived,
            CoreExt::onExtendedMessageReceipt,
            CoreExt::onExtendedMessageNegotiation,
            this,
            TOX_EXTENSION_MESSAGES_DEFAULT_MAX_RECEIVING_MESSAGE_SIZE),
        tox_extension_messages_free);
}

void CoreExt::process()
{
    std::lock_guard<std::mutex> lock(toxext_mutex);
    toxext_iterate(toxExt.get());
}

void CoreExt::onLosslessPacket(uint32_t friendId, const uint8_t* data, size_t length)
{
    if (is_toxext_packet(data, length)) {
        std::lock_guard<std::mutex> lock(toxext_mutex);
        toxext_handle_lossless_custom_packet(toxExt.get(), friendId, data, length);
    }
}

CoreExt::Packet::Packet(
    ToxExtPacketList* packetList_,
    ToxExtensionMessages* toxExtMessages_,
    uint32_t friendId_,
    std::mutex* toxext_mutex_,
    PacketPassKey passKey)
    : toxext_mutex(toxext_mutex_)
    , toxExtMessages(toxExtMessages_)
    , packetList(packetList_)
    , friendId(friendId_)
{
    std::ignore = passKey;
    assert(toxext_mutex != nullptr);
}

std::unique_ptr<ICoreExtPacket> CoreExt::getPacket(uint32_t friendId)
{
    return std::unique_ptr<Packet>(new Packet(
        toxext_packet_list_create(toxExt.get(), friendId),
        toxExtMessages.get(),
        friendId,
        &toxext_mutex,
        PacketPassKey{}));
}

uint64_t CoreExt::Packet::addExtendedMessage(QString message)
{
    if (hasBeenSent) {
        assert(false);
        qWarning() << "Invalid use of CoreExt::Packet";
        // Hope that UINT64_MAX will never collide with an actual receipt num
        // that we care about
        return UINT64_MAX;
    }

    int size = message.toUtf8().size();
    enum Tox_Extension_Messages_Error err;
    auto maxSize = static_cast<int>(tox_extension_messages_get_max_sending_size(
        toxExtMessages,
        friendId,
        &err));

    if (size > maxSize) {
        assert(false);
        qCritical() << "addExtendedMessage called with message of size:" << size
                    << "when max is:" << maxSize << ". Ignoring.";
        return false;
    }

    ToxString toxString(message);
    const auto receipt = tox_extension_messages_append(
        toxExtMessages,
        packetList,
        toxString.data(),
        toxString.size(),
        friendId,
        &err);

    if (err != TOX_EXTENSION_MESSAGES_SUCCESS) {
        qWarning() << "Error sending extension message";
    }

    return receipt;
}

bool CoreExt::Packet::send()
{
    std::lock_guard<std::mutex> lock(*toxext_mutex);

    auto ret = toxext_send(packetList);
    if (ret != TOXEXT_SUCCESS) {
        qWarning() << "Failed to send packet";
    }
    // Indicate we've sent the packet even on failure since our packetlist will
    // be invalid no matter what
    hasBeenSent = true;
    return ret == TOXEXT_SUCCESS;
}

uint64_t CoreExt::getMaxExtendedMessageSize()
{
    return TOX_EXTENSION_MESSAGES_DEFAULT_MAX_RECEIVING_MESSAGE_SIZE;
}

void CoreExt::onFriendStatusChanged(uint32_t friendId, Status::Status status)
{
    const auto prevStatusIt = currentStatuses.find(friendId);
    const auto prevStatus = prevStatusIt == currentStatuses.end()
        ? Status::Status::Offline : prevStatusIt->second;

    currentStatuses[friendId] = status;

    // Does not depend on prevStatus since prevStatus could be newly
    // constructed. In this case we should still ensure the rest of the system
    // knows there is no extension support
    if (status == Status::Status::Offline) {
        emit extendedMessageSupport(friendId, false);
    } else if (prevStatus == Status::Status::Offline) {
        tox_extension_messages_negotiate(toxExtMessages.get(), friendId);
    }
}

void CoreExt::onExtendedMessageReceived(uint32_t friendId, const uint8_t* data, size_t size, void* userData)
{
    QString msg = ToxString(data, size).getQString();
    emit static_cast<CoreExt*>(userData)->extendedMessageReceived(friendId, msg);
}

void CoreExt::onExtendedMessageReceipt(uint32_t friendId, uint64_t receiptId, void* userData)
{
    emit static_cast<CoreExt*>(userData)->extendedReceiptReceived(friendId, receiptId);
}

void CoreExt::onExtendedMessageNegotiation(uint32_t friendId, bool compatible, uint64_t maxMessageSize, void* userData)
{
    auto coreExt = static_cast<CoreExt*>(userData);

    // HACK: handling configurable max message size per-friend is not trivial.
    // For now the upper layers just assume that the max size for extended
    // messages is the same for all friends. If a friend has a max message size
    // lower than this value we just pretend they do not have the extension since
    // we will not split correctly for this friend.
    if (maxMessageSize < coreExt->getMaxExtendedMessageSize())
        compatible = false;

    emit coreExt->extendedMessageSupport(friendId, compatible);
}
