/*
    Copyright © 2019 by The qTox Project Contributors

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

#pragma once

#include <QDateTime>

#include <cstdint>
#include <memory>

/**
 * Abstraction around the toxext packet. The toxext flow is to allow several extensions
 * to tack onto the same packet before sending it to avoid needing the toxext overhead
 * for every single extension. This abstraction models a toxext packet list.
 *
 * Intent is to retrieve a ICoreExtPacket from an ICoreExtPacketAllocator, append all
 * relevant extension data, and then finally send the packet. After sending the packet
 * is no longer guaranteed to be valid.
 */
class ICoreExtPacket
{
public:
    ICoreExtPacket() = default;
    virtual ~ICoreExtPacket();
    ICoreExtPacket(const ICoreExtPacket&) = default;
    ICoreExtPacket& operator=(const ICoreExtPacket&) = default;
    ICoreExtPacket(ICoreExtPacket&&) = default;
    ICoreExtPacket& operator=(ICoreExtPacket&&) = default;
    /**
     * @brief Adds message to packet
     * @return Extended message receipt, UINT64_MAX on failure
     * @note Any other extensions related to this message have to be added
     * _before_ the message itself
     */
    virtual uint64_t addExtendedMessage(QString message) = 0;

    /**
     * @brief Consumes the packet constructed with PacketBuilder packet and
     * sends it to toxext
     */
    virtual bool send() = 0;
};

/**
 * Provider of toxext packets
 */
class ICoreExtPacketAllocator
{
public:
    virtual ~ICoreExtPacketAllocator();

    /**
     * @brief Gets a new packet builder for friend with core friend id friendId
     */
    virtual std::unique_ptr<ICoreExtPacket> getPacket(uint32_t friendId) = 0;
};
