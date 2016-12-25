/*
    Copyright © 2015 by The qTox Project Contributors

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

// Tox doesn't publicly define these
#define NOSPAM_BYTES                4
#define CHECKSUM_BYTES              2

#define PUBLIC_KEY_HEX_CHARS        (2*TOX_PUBLIC_KEY_SIZE)
#define NOSPAM_HEX_CHARS            (2*NOSPAM_BYTES)
#define CHECKSUM_HEX_CHARS          (2*CHECKSUM_BYTES)
#define TOXID_HEX_CHARS             (2*TOX_ADDRESS_SIZE)

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
: toxId()
{}

/**
 * @brief The copy constructor.
 * @param other ToxId to copy
 */
ToxId::ToxId(const ToxId& other)
: toxId(other.toxId)
{}

/**
 * @brief Create a Tox ID from a QString.
 *
 * If the given id is not a valid Tox ID, then:
 * publicKey == id and noSpam == "" == checkSum.
 *
 * @param id Tox ID string to convert to ToxId object
 */
ToxId::ToxId(const QString& id)
{
    if (isToxId(id))
    {
        toxId = QByteArray::fromHex(id.toLatin1());
    }
    else if(id.length() >= PUBLIC_KEY_HEX_CHARS)
    {
        toxId = QByteArray::fromHex(id.left(PUBLIC_KEY_HEX_CHARS).toLatin1());
    }
    else
    {
        toxId = QByteArray(TOX_ADDRESS_SIZE, 0x00); // invalid id string
    }
}

/**
 * @brief Create a Tox ID from a QByteArray.
 *
 * If the given id is not a valid Tox ID, then:
 * publicKey == id and noSpam == "" == checkSum.
 *
 * @param id Tox ID string to convert to ToxId object
 */
ToxId::ToxId(const QByteArray& rawId)
{
    checkToxId(rawId);
}

/**
 * @brief Create a Tox ID from a uint8_t* and length.
 *
 * If the given id is not a valid Tox ID, then:
 * publicKey == id and noSpam == "" == checkSum.
 *
 * @param id Tox ID string to convert to ToxId object
 */
ToxId::ToxId(const uint8_t& rawId, int len)
{
    QByteArray tmpId(reinterpret_cast<const char *>(rawId), len);
    checkToxId(tmpId);
}


void ToxId::checkToxId(const QByteArray& rawId)
{
    if(rawId.length() == TOX_SECRET_KEY_SIZE)
    {
        toxId = QByteArray(rawId);                  // construct from PK only
    }
    else if (rawId.length() == TOX_ADDRESS_SIZE
             && isToxId(rawId.toHex().toUpper()))

    {
        toxId = QByteArray(rawId);                  // construct from full toxid
    }
    else
    {
        toxId = QByteArray(TOX_ADDRESS_SIZE, 0x00); // invalid id string
    }
}

/**
 * @brief Compares, that public key equals.
 * @param other Tox ID to compare.
 * @return True if both Tox ID have same public keys, false otherwise.
 */
bool ToxId::operator==(const ToxId& other) const
{
    return getPublicKey() == other.getPublicKey();
}

/**
 * @brief Compares, that only public key not equals.
 * @param other Tox ID to compare.
 * @return True if both Tox ID have different public keys, false otherwise.
 */
bool ToxId::operator!=(const ToxId& other) const
{
    return getPublicKey() != other.getPublicKey();
}

/**
 * @brief Returns Tox ID converted to QString.
 * @return The Tox ID as QString.
 */
QString ToxId::toString() const
{
    return toxId.toHex().toUpper();
}

/**
 * @brief Clears all elements of the Tox ID.
 */
void ToxId::clear()
{
    toxId.clear();
}

/**
 * @brief Check, that id is probably a valid Tox ID.
 * @param id Tox ID to check.
 * @return True if the string can be a ToxID, false otherwise.
 * @note Doesn't validate checksum.
 */
bool ToxId::isValidToxId(const QString& id)
{
    const QRegularExpression hexRegExp("^[A-Fa-f0-9]+$");
    return id.length() == TOXID_HEX_CHARS && id.contains(hexRegExp);
}

/**
 * @brief Gets the ToxID as bytes, convenience function for toxcore interface.
 * @return The ToxID
 */
const uint8_t* ToxId::getBytes() const
{
    return reinterpret_cast<const uint8_t*> (toxId.constData());
}

/**
 * @brief Gets the Public Key part of the ToxID
 * @return Public Key of the ToxID
 */
QByteArray ToxId::getPublicKey() const
{
    return toxId.mid(0, TOX_PUBLIC_KEY_SIZE);
}

/**
 * @brief Gets the Public Key part of the ToxID, convenience fuction for toxcore interface.
 * @return Public Key of the ToxID
 */
const uint8_t* ToxId::getPublicKeyBytes() const
{
    return reinterpret_cast<const uint8_t*> (toxId.mid(0, TOX_PUBLIC_KEY_SIZE).constData());
}

/**
 * @brief Returns the Public Key converted to QString.
 * @return The Public Key as QString.
 */
QString ToxId::getPublicKeyString() const
{
    return getPublicKey().toHex().toUpper();
}

/**
 * @brief Returns the NoSpam value converted to QString.
 * @return The NoSpam value as QString or "" if the ToxId was constructed from a Public Key.
 */
QString ToxId::getNoSpamString() const
{
    if(toxId.length() == TOX_ADDRESS_SIZE)
    {
        return toxId.mid(TOX_PUBLIC_KEY_SIZE, NOSPAM_BYTES).toHex().toUpper();
    }

    return {};
}

/**
 * @brief Check, that id is a valid Tox ID.
 * @param id Tox ID to check.
 * @return True if id is a valid Tox ID, false otherwise.
 */
bool ToxId::isToxId(const QString& id)
{
    if (!isValidToxId(id))
    {
        return false;
    }

    uint32_t size = PUBLIC_KEY_HEX_CHARS + NOSPAM_HEX_CHARS;
    QString publicKeyStr = id.left(size);
    QString checksumStr = id.right(CHECKSUM_HEX_CHARS);

    QByteArray publicKey = QByteArray::fromHex(publicKeyStr.toLatin1());
    QByteArray checksum = QByteArray::fromHex(checksumStr.toLatin1());
    uint8_t check[2] = {0};

    for (uint32_t i = 0; i < size / 2; i++)
    {
        check[i % 2] ^= publicKey.data()[i];
    }

    QByteArray caclulated(reinterpret_cast<char*>(check), 2);
    return caclulated == checksum;
 }
