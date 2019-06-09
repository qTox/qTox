#ifndef GROUPID_H
#define GROUPID_H

#include "src/core/contactid.h"
#include <QByteArray>
#include <cstdint>

class GroupId : public ContactId
{
public:
    GroupId();
    GroupId(const GroupId& other);
    explicit GroupId(const QByteArray& rawId);
    explicit GroupId(const uint8_t* rawId);
    int getSize() const override;
};

#endif // GROUPID_H
