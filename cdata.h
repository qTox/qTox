#ifndef CDATA_H
#define CDATA_H

#include <cstdint>
#include <QString>
#include "tox/tox.h"

class CData
{
public:
    uint8_t* data();
    uint16_t size();

protected:
    explicit CData(const QString& data, uint16_t byteSize);
    virtual ~CData();

    static QString toString(const uint8_t* cData, const uint16_t cDataSize);

private:
    uint8_t* cData;
    uint16_t cDataSize;

    static uint16_t fromString(const QString& userId, uint8_t* cData);
};

class CUserId : public CData
{
public:
    explicit CUserId(const QString& userId);

    static QString toString(const uint8_t *cUserId);

private:
    static const uint16_t SIZE = TOX_CLIENT_ID_SIZE;

};

class CFriendAddress : public CData
{
public:
    explicit CFriendAddress(const QString& friendAddress);

    static QString toString(const uint8_t* cFriendAddress);

private:
    static const uint16_t SIZE = TOX_FRIEND_ADDRESS_SIZE;

};

#endif // CDATA_H
