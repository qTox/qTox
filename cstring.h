#ifndef CSTRING_H
#define CSTRING_H

#include <cstdint>
#include <QString>

class CString
{
public:
    explicit CString(const QString& string);
    ~CString();

    uint8_t* data();
    uint16_t size();

    static QString toString(const uint8_t* cMessage, const uint16_t cMessageSize);

private:
    const static int MAX_SIZE_OF_UTF8_ENCODED_CHARACTER = 4;

    uint8_t* cString;
    uint16_t cStringSize;

    static uint16_t fromString(const QString& message, uint8_t* cMessage);
};
#endif // CSTRING_H
