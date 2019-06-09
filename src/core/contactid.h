#ifndef CONTACTID_H
#define CONTACTID_H

#include <QByteArray>
#include <QString>
#include <cstdint>
#include <QHash>
#include <memory>

class ContactId
{
public:
    virtual ~ContactId() = default;
    ContactId& operator=(const ContactId& other) = default;
    ContactId& operator=(ContactId&& other) = default;
    bool operator==(const ContactId& other) const;
    bool operator!=(const ContactId& other) const;
    bool operator<(const ContactId& other) const;
    QString toString() const;
    QByteArray getByteArray() const;
    const uint8_t* getData() const;
    bool isEmpty() const;
    virtual int getSize() const = 0;

protected:
    ContactId();
    explicit ContactId(const QByteArray& rawId);
    QByteArray id;
};

inline uint qHash(const ContactId& id)
{
    return qHash(id.getByteArray());
}

using ContactIdPtr = std::shared_ptr<const ContactId>;

#endif // CONTACTID_H
