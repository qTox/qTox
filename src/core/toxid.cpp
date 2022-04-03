/*
    Copyright © 2015-2019 by The qTox Project Contributors

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
#include "toxid.h"
#include "toxpk.h"

#include <QRegularExpression>

#include <cassert>
#include <cstdint>

const QRegularExpression ToxId::ToxIdRegEx(QString("(^|\\s)[A-Fa-f0-9]{%1}($|\\s)").arg(ToxId::numHexChars));

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
 * If the given rawId is not a valid Tox ID, but can be a Public Key then:
 * publicKey == rawId and noSpam == 0 == checkSum.
 * If the given rawId isn't a valid Public Key or Tox ID a ToxId with all zero bytes is created.
 *
 * @param id Tox ID string to convert to ToxId object
 */
ToxId::ToxId(const QString& id)
{
    if (isToxId(id)) {
        toxId = QByteArray::fromHex(id.toLatin1());
    } else {
        assert(!"ToxId constructed with invalid length string");
        toxId = QByteArray(); // invalid id string
    }
}

/**
 * @brief Create a Tox ID from a QByteArray.
 *
 * If the given rawId is not a valid Tox ID, but can be a Public Key then:
 * publicKey == rawId and noSpam == 0 == checkSum.
 * If the given rawId isn't a valid Public Key or Tox ID a ToxId with all zero bytes is created.
 *
 * @param rawId Tox ID bytes to convert to ToxId object
 */
ToxId::ToxId(const QByteArray& rawId)
{
    constructToxId(rawId);
}

/**
 * @brief Create a Tox ID from uint8_t bytes and lenght, convenience function for toxcore interface.
 *
 * If the given rawId is not a valid Tox ID, but can be a Public Key then:
 * publicKey == rawId and noSpam == 0 == checkSum.
 * If the given rawId isn't a valid Public Key or Tox ID a ToxId with all zero bytes is created.
 *
 * @param rawId Pointer to bytes to convert to ToxId object
 * @param len Number of bytes to read. Must be ToxPk::size for a Public Key or
 *            ToxId::size for a Tox ID.
 */
ToxId::ToxId(const uint8_t* rawId, int len)
{
    QByteArray tmpId(reinterpret_cast<const char*>(rawId), len);
    constructToxId(tmpId);
}


void ToxId::constructToxId(const QByteArray& rawId)
{
    if (rawId.length() == ToxId::size && isToxId(QString::fromUtf8(rawId.toHex()).toUpper())) {
        toxId = QByteArray(rawId); // construct from full toxid
    } else {
        assert(!"ToxId constructed with invalid input");
        toxId = QByteArray(); // invalid id
    }
}

/**
 * @brief Compares the equality of the Public Key.
 * @param other Tox ID to compare.
 * @return True if both Tox IDs have the same public keys, false otherwise.
 */
bool ToxId::operator==(const ToxId& other) const
{
    return getPublicKey() == other.getPublicKey();
}

/**
 * @brief Compares the inequality of the Public Key.
 * @param other Tox ID to compare.
 * @return True if both Tox IDs have different public keys, false otherwise.
 */
bool ToxId::operator!=(const ToxId& other) const
{
    return getPublicKey() != other.getPublicKey();
}

/**
 * @brief Returns the Tox ID converted to QString.
 * Is equal to getPublicKey() if the Tox ID was constructed from only a Public Key.
 * @return The Tox ID as QString.
 */
QString ToxId::toString() const
{
    return QString::fromUtf8(toxId.toHex()).toUpper();
}

/**
 * @brief Clears all elements of the Tox ID.
 */
void ToxId::clear()
{
    toxId.clear();
}

/**
 * @brief Gets the ToxID as bytes, convenience function for toxcore interface.
 * @return The ToxID as uint8_t* if isValid() is true, else a nullptr.
 */
const uint8_t* ToxId::getBytes() const
{
    if (isValid()) {
        return reinterpret_cast<const uint8_t*>(toxId.constData());
    }

    return nullptr;
}

/**
 * @brief Gets the Public Key part of the ToxID
 * @return Public Key of the ToxID
 */
ToxPk ToxId::getPublicKey() const
{
    auto const pkBytes = toxId.left(ToxPk::size);
    if (pkBytes.isEmpty()) {
        return ToxPk{};
    } else {
        return ToxPk{pkBytes};
    }
}

/**
 * @brief Returns the NoSpam value converted to QString.
 * @return The NoSpam value as QString or "" if the ToxId was constructed from a Public Key.
 */
QString ToxId::getNoSpamString() const
{
    if (toxId.length() == ToxId::size) {
        return QString::fromUtf8(toxId.mid(ToxPk::size, ToxId::nospamSize).toHex()).toUpper();
    }

    return {};
}

/**
 * @brief Check, that id is a valid Tox ID.
 * @param id Tox ID to check.
 * @return True if id is a valid Tox ID, false otherwise.
 * @note Validates the checksum.
 */
bool ToxId::isValidToxId(const QString& id)
{
    return isToxId(id) && ToxId(id).isValid();
}

/**
 * @brief Check, that id is probably a valid Tox ID.
 * @param id Tox ID to check.
 * @return True if the string can be a ToxID, false otherwise.
 * @note Doesn't validate checksum.
 */
bool ToxId::isToxId(const QString& id)
{
    return id.length() == ToxId::numHexChars && id.contains(ToxIdRegEx);
}

/**
 * @brief Check it it's a valid Tox ID by verifying the checksum
 * @return True if it is a valid Tox ID, false otherwise.
 */
bool ToxId::isValid() const
{
    if (toxId.length() != ToxId::size) {
        return false;
    }

    const int pkAndChecksum = ToxPk::size + ToxId::nospamSize;

    QByteArray data = toxId.left(pkAndChecksum);
    QByteArray checksum = toxId.right(ToxId::checksumSize);
    QByteArray calculated(ToxId::checksumSize, 0x00);

    for (int i = 0; i < pkAndChecksum; i++) {
        calculated[i % 2] = calculated[i % 2] ^ data[i];
    }

    return calculated == checksum;
}
