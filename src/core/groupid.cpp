#include "groupid.h"

#include <tox/tox.h>

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
    : ContactId()
{
}

/**
 * @brief The copy constructor.
 * @param other GroupId to copy
 */
GroupId::GroupId(const GroupId& other)
    : ContactId(other.id)
{
}

/**
 * @brief Constructs a GroupId from bytes.
 * @param rawId The bytes to construct the GroupId from. The lenght must be exactly
 *              TOX_CONFERENCE_UID_SIZE, else the GroupId will be empty.
 */
GroupId::GroupId(const QByteArray& rawId)
    : ContactId([rawId](){
        assert(rawId.length() == TOX_CONFERENCE_UID_SIZE);
        return rawId;}())
{
}

/**
 * @brief Constructs a GroupId from bytes.
 * @param rawId The bytes to construct the GroupId from, will read exactly
 * TOX_CONFERENCE_UID_SIZE from the specified buffer.
 */
GroupId::GroupId(const uint8_t* rawId)
    : ContactId(QByteArray(reinterpret_cast<const char*>(rawId), TOX_CONFERENCE_UID_SIZE))
{
}

/**
 * @brief Get size of public id in bytes.
 * @return Size of public id in bytes.
 */
int GroupId::getSize() const
{
    return TOX_CONFERENCE_UID_SIZE;
}
