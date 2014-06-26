#include "cdata.h"

// CData

CData::CData(const QString &data, uint16_t byteSize)
{
    cData = new uint8_t[byteSize];
    cDataSize = fromString(data, cData);
}

CData::~CData()
{
    delete[] cData;
}

uint8_t* CData::data()
{
    return cData;
}

uint16_t CData::size()
{
    return cDataSize;
}

QString CData::toString(const uint8_t *cData, const uint16_t cDataSize)
{
    return QString(QByteArray(reinterpret_cast<const char*>(cData), cDataSize).toHex()).toUpper();
}

uint16_t CData::fromString(const QString& data, uint8_t* cData)
{
    QByteArray arr = QByteArray::fromHex(data.toLower().toLatin1());
    memcpy(cData, reinterpret_cast<uint8_t*>(arr.data()), arr.size());
    return arr.size();
}


// CUserId

CUserId::CUserId(const QString &userId) :
    CData(userId, SIZE)
{
    // intentionally left empty
}

QString CUserId::toString(const uint8_t* cUserId)
{
    return CData::toString(cUserId, SIZE);
}


// CFriendAddress

CFriendAddress::CFriendAddress(const QString &friendAddress) :
    CData(friendAddress, SIZE)
{
    // intentionally left empty
}

QString CFriendAddress::toString(const uint8_t *cFriendAddress)
{
    return CData::toString(cFriendAddress, SIZE);
}
