#include "toxpk.h"

#include <tox/tox.h>

#include <QByteArray>
#include <QString>

/**
 * @class ToxPk
 * @brief This class represents a Tox Public Key, which is a part of Tox ID.
 */

/**
 * @brief The default constructor. Creates an empty Tox key.
 */
ToxPk::ToxPk()
: key()
{}

/**
 * @brief The copy constructor.
 * @param other ToxKey to copy
 */
ToxPk::ToxPk(const ToxPk& other)
: key(other.key)
{}

/**
 * @brief Constructs a ToxKey from bytes.
 * @param rawId The bytes to construct the ToxKey from. The lenght must be exactly
 *              TOX_PUBLIC_KEY_SIZE, else the ToxKey will be empty.
 */
ToxPk::ToxPk(const QByteArray& rawId)
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
 * @param rawId The bytes to construct the ToxKey from, will read exactly
 * TOX_PUBLIC_KEY_SIZE from the specified buffer.
 */
ToxPk::ToxPk(const uint8_t* rawId)
{
    key = QByteArray(reinterpret_cast<const char*>(rawId), TOX_PUBLIC_KEY_SIZE);
}

/**
 * @brief Compares the equality of the ToxKey.
 * @param other ToxKey to compare.
 * @return True if both ToxKeys are equal, false otherwise.
 */
bool ToxPk::operator==(const ToxPk& other) const
{
    return key == other.key;
}

/**
 * @brief Compares the inequality of the ToxKey.
 * @param other ToxKey to compare.
 * @return True if both ToxKeys are not equal, false otherwise.
 */
bool ToxPk::operator!=(const ToxPk& other) const
{
    return key != other.key;
}

/**
 * @brief Converts the ToxKey to a uppercase hex string.
 * @return QString containing the hex representation of the key
 */
QString ToxPk::toString() const
{
    return key.toHex().toUpper();
}

/**
 * @brief Returns a pointer to the raw key data.
 * @return Pointer to the raw key data, which is exactly TOX_PUBLIC_KEY_SIZE bytes
 *         long. Returns a nullptr if the ToxKey is empty.
 */
const uint8_t* ToxPk::getBytes() const
{
    if(key.isEmpty())
    {
        return nullptr;
    }

    return reinterpret_cast<const uint8_t*>(key.constData());
}

/**
 * @brief Checks if the ToxKey contains a key.
 * @return True if there is a key, False otherwise.
 */
bool ToxPk::isEmpty() const
{
    return key.isEmpty();
}
