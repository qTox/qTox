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
{
}

/**
 * @brief The copy constructor.
 * @param other ToxPk to copy
 */
ToxPk::ToxPk(const ToxPk& other)
    : key(other.key)
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from. The lenght must be exactly
 *              TOX_PUBLIC_KEY_SIZE, else the ToxPk will be empty.
 */
ToxPk::ToxPk(const QByteArray& rawId)
{
    if (rawId.length() == TOX_PUBLIC_KEY_SIZE) {
        key = QByteArray(rawId);
    } else {
        key = QByteArray();
    }
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from, will read exactly
 * TOX_PUBLIC_KEY_SIZE from the specified buffer.
 */
ToxPk::ToxPk(const uint8_t* rawId)
{
    key = QByteArray(reinterpret_cast<const char*>(rawId), TOX_PUBLIC_KEY_SIZE);
}

/**
 * @brief Compares the equality of the ToxPk.
 * @param other ToxPk to compare.
 * @return True if both ToxPks are equal, false otherwise.
 */
bool ToxPk::operator==(const ToxPk& other) const
{
    return key == other.key;
}

/**
 * @brief Compares the inequality of the ToxPk.
 * @param other ToxPk to compare.
 * @return True if both ToxPks are not equal, false otherwise.
 */
bool ToxPk::operator!=(const ToxPk& other) const
{
    return key != other.key;
}

/**
 * @brief Converts the ToxPk to a uppercase hex string.
 * @return QString containing the hex representation of the key
 */
QString ToxPk::toString() const
{
    return key.toHex().toUpper();
}

/**
 * @brief Returns a pointer to the raw key data.
 * @return Pointer to the raw key data, which is exactly `ToxPk::getPkSize()`
 *         bytes long. Returns a nullptr if the ToxPk is empty.
 */
const uint8_t* ToxPk::getBytes() const
{
    if (key.isEmpty()) {
        return nullptr;
    }

    return reinterpret_cast<const uint8_t*>(key.constData());
}

/**
 * @brief Get a copy of the key
 * @return Copied key bytes
 */
QByteArray ToxPk::getKey() const
{
    return QByteArray(key); // TODO: Is a copy really necessary?
}

/**
 * @brief Checks if the ToxPk contains a key.
 * @return True if there is a key, False otherwise.
 */
bool ToxPk::isEmpty() const
{
    return key.isEmpty();
}

/**
 * @brief Get size of public key in bytes.
 * @return Size of public key in bytes.
 */
int ToxPk::getPkSize()
{
    return TOX_PUBLIC_KEY_SIZE;
}
