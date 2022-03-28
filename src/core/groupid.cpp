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

#include "groupid.h"

#include <QByteArray>
#include <QString>

#include <cassert>

/**
 * @class GroupId
 * @brief This class represents a long term persistent group identifier.
 */

/**
 * @brief The default constructor. Creates an empty Tox group ID.
 */
GroupId::GroupId()
    : ChatId()
{
}

/**
 * @brief Constructs a GroupId from bytes.
 * @param rawId The bytes to construct the GroupId from. The lenght must be exactly
 *              GroupId::size, else the GroupId will be empty.
 */
GroupId::GroupId(const QByteArray& rawId)
    : ChatId([rawId](){
        assert(rawId.length() == size);
        return rawId;}())
{
}

/**
 * @brief Constructs a GroupId from bytes.
 * @param rawId The bytes to construct the GroupId from, will read exactly
 * GroupId::size from the specified buffer.
 */
GroupId::GroupId(const uint8_t* rawId)
    : ChatId(QByteArray(reinterpret_cast<const char*>(rawId), size))
{
}

/**
 * @brief Get size of public id in bytes.
 * @return Size of public id in bytes.
 */
int GroupId::getSize() const
{
    return size;
}

std::unique_ptr<ChatId> GroupId::clone() const
{
    return std::unique_ptr<ChatId>(new GroupId(*this));
}
