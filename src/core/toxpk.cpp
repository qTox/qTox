#include "toxpk.h"

#include <tox/tox.h>

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
    : ContactId()
{
}

/**
 * @brief The copy constructor.
 * @param other ToxPk to copy
 */
ToxPk::ToxPk(const ToxPk& other)
    : ContactId(other.id)
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from. The lenght must be exactly
 *              TOX_PUBLIC_KEY_SIZE, else the ToxPk will be empty.
 */
ToxPk::ToxPk(const QByteArray& rawId)
    : ContactId([rawId](){
        assert(rawId.length() == TOX_PUBLIC_KEY_SIZE);
        return rawId;}())
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from, will read exactly
 * TOX_PUBLIC_KEY_SIZE from the specified buffer.
 */
ToxPk::ToxPk(const uint8_t* rawId)
    : ContactId(QByteArray(reinterpret_cast<const char*>(rawId), TOX_PUBLIC_KEY_SIZE))
{
}

/**
 * @brief Get size of public key in bytes.
 * @return Size of public key in bytes.
 */
int ToxPk::getSize() const
{
    return TOX_PUBLIC_KEY_SIZE;
}
