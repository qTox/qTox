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

#include "chatid.h"
#include "toxpk.h"

#include <QByteArray>
#include <QString>

#include <cassert>

/**
 * @class ToxPk
 * @brief This class represents a Tox Public Key, which is a part of Tox ID.
 */

/**
 * @brief The default constructor. Creates an empty Tox key.
 */
ToxPk::ToxPk()
    : ChatId()
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from. The lenght must be exactly
 *              ToxPk::size, else the ToxPk will be empty.
 */
ToxPk::ToxPk(const QByteArray& rawId)
    : ChatId([&rawId](){
        assert(rawId.length() == size);
        return rawId;}())
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from, will read exactly
 * ToxPk::size from the specified buffer.
 */
ToxPk::ToxPk(const uint8_t* rawId)
    : ChatId(QByteArray(reinterpret_cast<const char*>(rawId), size))
{
}

/**
 * @brief Constructs a ToxPk from a QString.
 *
  * If the given pk isn't a valid Public Key a ToxPk with all zero bytes is created.
 *
 * @param pk Tox Pk string to convert to ToxPk object
 */
ToxPk::ToxPk(const QString& pk)
    : ChatId([&pk](){
    if (pk.length() == numHexChars) {
        return QByteArray::fromHex(pk.toLatin1());
    } else {
        assert(!"ToxPk constructed with invalid length string");
        return QByteArray(); // invalid pk string
    }
    }())
{
}

/**
 * @brief Get size of public key in bytes.
 * @return Size of public key in bytes.
 */
int ToxPk::getSize() const
{
    return size;
}

std::unique_ptr<ChatId> ToxPk::clone() const
{
    return std::unique_ptr<ChatId>(new ToxPk(*this));
}
