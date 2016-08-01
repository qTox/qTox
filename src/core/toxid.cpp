/*
    Copyright © 2015 by The qTox Project

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


#include "toxid.h"

#include "core.h"

#include <tox/tox.h>
#include <qregularexpression.h>

#define TOX_ID_PUBLIC_KEY_LENGTH 64
#define TOX_ID_NO_SPAM_LENGTH    8
#define TOX_ID_CHECKSUM_LENGTH   4
#define TOX_HEX_ID_LENGTH 2*TOX_ADDRESS_SIZE

/**
 * @class ToxId
 * @brief This class represents a Tox ID.
 *
 * An ID is composed of 32 bytes long public key, 4 bytes long NoSpam
 * and 2 bytes long checksum.
 *
 * e.g.
 * @code
 * | C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30 | C8BA3AB9 | BEB9
 * |                                                                 /           |
 * |                                                                /    NoSpam  | Checksum
 * |           Public Key (PK), 32 bytes, 64 characters            /    4 bytes  |  2 bytes
 * |                                                              |  8 characters|  4 characters
 * @endcode
 */

/**
 * @brief The default constructor. Creates an empty Tox ID.
 */
ToxId::ToxId()
: publicKey(), noSpam(), checkSum()
{}

/**
 * @brief The copy constructor.
 * @param other ToxId to copy
 */
ToxId::ToxId(const ToxId &other)
: publicKey(other.publicKey), noSpam(other.noSpam), checkSum(other.checkSum)
{}

/**
 * @brief Create a Tox ID from QString.
 *
 * If the given id is not a valid Tox ID, then:
 * publicKey == id and noSpam == "" == checkSum.
 *
 * @param id Tox ID string to convert to ToxId object
 */
ToxId::ToxId(const QString &id)
{
    if (isToxId(id))
    {
        publicKey = id.left(TOX_ID_PUBLIC_KEY_LENGTH);
        noSpam    = id.mid(TOX_ID_PUBLIC_KEY_LENGTH, TOX_ID_NO_SPAM_LENGTH);
        checkSum  = id.mid(TOX_ID_PUBLIC_KEY_LENGTH + TOX_ID_NO_SPAM_LENGTH, TOX_ID_CHECKSUM_LENGTH);
    }
    else
    {
        publicKey = id;
    }
}

/**
 * @brief Compares, that public key equals.
 * @param other Tox ID to compare.
 * @return True if both Tox ID have same public keys, false otherwise.
 */
bool ToxId::operator==(const ToxId& other) const
{
    return publicKey == other.publicKey;
}

/**
 * @brief Compares, that only public key not equals.
 * @param other Tox ID to compare.
 * @return True if both Tox ID have different public keys, false otherwise.
 */
bool ToxId::operator!=(const ToxId &other) const
{
    return publicKey != other.publicKey;
}

/**
 * @brief Check, that the current user ID is the active user ID
 * @return True if this Tox ID is equals to
 * the Tox ID of the currently active profile.
 */
bool ToxId::isSelf() const
{
    return *this == Core::getInstance()->getSelfId();
}

/**
 * @brief Returns Tox ID converted to QString.
 * @return The Tox ID as QString.
 */
QString ToxId::toString() const
{
    return publicKey + noSpam + checkSum;
}

/**
 * @brief Clears all elements of the Tox ID.
 */
void ToxId::clear()
{
    publicKey.clear();
    noSpam.clear();
    checkSum.clear();
}

/**
 * @brief Check, that id is a valid Tox ID.
 * @param id Tox ID to check.
 * @return True if id is a valid Tox ID, false otherwise.
 */
bool ToxId::isToxId(const QString &id)
{
    const QRegularExpression hexRegExp("^[A-Fa-f0-9]+$");
    return id.length() == TOX_HEX_ID_LENGTH && id.contains(hexRegExp);
}
