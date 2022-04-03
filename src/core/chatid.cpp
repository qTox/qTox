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

#include <QByteArray>
#include <QString>
#include <cstdint>
#include <QHash>
#include "src/core/chatid.h"

/**
 * @brief The default constructor. Creates an empty id.
 */
ChatId::ChatId()
    : id()
{
}
ChatId::~ChatId() = default;

/**
 * @brief Constructs a ChatId from bytes.
 * @param rawId The bytes to construct the ChatId from.
 */
ChatId::ChatId(const QByteArray& rawId)
{
    id = QByteArray(rawId);
}

/**
 * @brief Compares the equality of the ChatId.
 * @param other ChatId to compare.
 * @return True if both ChatId are equal, false otherwise.
 */
bool ChatId::operator==(const ChatId& other) const
{
    return id == other.id;
}

/**
 * @brief Compares the inequality of the ChatId.
 * @param other ChatId to compare.
 * @return True if both ChatIds are not equal, false otherwise.
 */
bool ChatId::operator!=(const ChatId& other) const
{
    return id != other.id;
}

/**
 * @brief Compares two ChatIds
 * @param other ChatId to compare.
 * @return True if this ChatIds is less than the other ChatId, false otherwise.
 */
bool ChatId::operator<(const ChatId& other) const
{
    return id < other.id;
}

/**
 * @brief Converts the ChatId to a uppercase hex string.
 * @return QString containing the hex representation of the id
 */
QString ChatId::toString() const
{
    return QString::fromUtf8(id.toHex()).toUpper();
}

/**
 * @brief Returns a pointer to the raw id data.
 * @return Pointer to the raw id data, which is exactly `ChatId::getPkSize()`
 *         bytes long. Returns a nullptr if the ChatId is empty.
 */
const uint8_t* ChatId::getData() const
{
    if (id.isEmpty()) {
        return nullptr;
    }

    return reinterpret_cast<const uint8_t*>(id.constData());
}

/**
 * @brief Get a copy of the id
 * @return Copied id bytes
 */
QByteArray ChatId::getByteArray() const
{
    return QByteArray(id); // TODO: Is a copy really necessary?
}

/**
 * @brief Checks if the ChatId contains a id.
 * @return True if there is a id, False otherwise.
 */
bool ChatId::isEmpty() const
{
    return id.isEmpty();
}
