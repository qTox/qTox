#include <QByteArray>
#include <QString>
#include <cstdint>
#include <QHash>
#include "src/core/contactid.h"

/**
 * @brief The default constructor. Creates an empty id.
 */
ContactId::ContactId()
    : id()
{
}

/**
 * @brief Constructs a ContactId from bytes.
 * @param rawId The bytes to construct the ContactId from.
 */
ContactId::ContactId(const QByteArray& rawId)
{
    id = QByteArray(rawId);
}

/**
 * @brief Compares the equality of the ContactId.
 * @param other ContactId to compare.
 * @return True if both ContactId are equal, false otherwise.
 */
bool ContactId::operator==(const ContactId& other) const
{
    return id == other.id;
}

/**
 * @brief Compares the inequality of the ContactId.
 * @param other ContactId to compare.
 * @return True if both ContactIds are not equal, false otherwise.
 */
bool ContactId::operator!=(const ContactId& other) const
{
    return id != other.id;
}

/**
 * @brief Compares two ContactIds
 * @param other ContactId to compare.
 * @return True if this ContactIds is less than the other ContactId, false otherwise.
 */
bool ContactId::operator<(const ContactId& other) const
{
    return id < other.id;
}

/**
 * @brief Converts the ContactId to a uppercase hex string.
 * @return QString containing the hex representation of the id
 */
QString ContactId::toString() const
{
    return id.toHex().toUpper();
}

/**
 * @brief Returns a pointer to the raw id data.
 * @return Pointer to the raw id data, which is exactly `ContactId::getPkSize()`
 *         bytes long. Returns a nullptr if the ContactId is empty.
 */
const uint8_t* ContactId::getData() const
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
QByteArray ContactId::getByteArray() const
{
    return QByteArray(id); // TODO: Is a copy really necessary?
}

/**
 * @brief Checks if the ContactId contains a id.
 * @return True if there is a id, False otherwise.
 */
bool ContactId::isEmpty() const
{
    return id.isEmpty();
}
