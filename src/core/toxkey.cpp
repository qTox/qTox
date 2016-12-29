#include "toxkey.h"

#include <tox/tox.h>

#include <QByteArray>
#include <QString>

/**
 * @class ToxKey
 * @brief This class represents a Tox Public Key, which is a part of Tox ID.
 */

/**
 * @brief The default constructor. Creates an empty Tox key.
 */
ToxKey::ToxKey()
: key()
{}

/**
 * @brief The copy constructor.
 * @param other ToxKey to copy
 */
ToxKey::ToxKey(const ToxKey &other)
: key(other.key)
{}

/**
 * @brief Constructs a ToxKey from bytes.
 * @param rawId The bytes to construct the ToxKey from. The lenght must be exactly
 *              TOX_PUBLIC_KEY_SIZE, else the ToxKey will be empty.
 */
ToxKey::ToxKey(const QByteArray &rawId)
{
    if(rawId.length() == TOX_PUBLIC_KEY_SIZE)
    {
        key = QByteArray(rawId);
    }
    else
    {
        key = QByteArray();
    }
}

/**
 * @brief Constructs a ToxKey from bytes.
 * @param rawId The bytes to construct the ToxKey from.
 * @param len Number of bytes to read. Must be exactly TOX_PUBLIC_KEY_SIZE, else the ToxKey will be empty.
 */
ToxKey::ToxKey(const uint8_t *rawId, int len)
{
    if(len == TOX_PUBLIC_KEY_SIZE)
    {
        key = QByteArray(reinterpret_cast<const char*>(rawId), len);
    }
    else
    {
        key = QByteArray();
    }
}

/**
 * @brief Compares the equality of the ToxKey.
 * @param other ToxKey to compare.
 * @return True if both ToxKeys are equal, false otherwise.
 */
bool ToxKey::operator==(const ToxKey &other) const
{
    return key == other.key;
}

/**
 * @brief Compares the inequality of the ToxKey.
 * @param other ToxKey to compare.
 * @return True if both ToxKeys are not equal, false otherwise.
 */
bool ToxKey::operator!=(const ToxKey &other) const
{
    return key != other.key;
}

/**
 * @brief Converts the ToxKey to a uppercase hex string.
 * @return QString containing the hex representation of the key
 */
QString ToxKey::toString() const
{
    return key.toHex().toUpper();
}

/**
 * @brief Returns a pointer to the raw key data.
 * @return Pointer to the raw key data, which is exactly TOX_PUBLIC_KEY_SIZE bytes
 *         long. Returns a nullptr if the ToxKey is empty.
 */
const uint8_t *ToxKey::getBytes() const
{
    if(key.isEmpty())
    {
        return nullptr;
    }

    return reinterpret_cast<const uint8_t*>(key.constData());
}

/**
 * @brief Get a copy of the key
 * @return Copied key bytes
 */
QByteArray ToxKey::getKey() const
{
    return QByteArray(key); // TODO: Is a copy really necessary?
}

/**
 * @brief Checks if the ToxKey contains a key.
 * @return True if there is a key, False otherwise.
 */
bool ToxKey::isEmpty() const
{
    return key.isEmpty();
}
