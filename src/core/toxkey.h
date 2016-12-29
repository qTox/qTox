#ifndef TOXKEY_H
#define TOXKEY_H

#include <cstdint>
#include <QByteArray>
#include <QString>

class ToxKey
{
public:
    ToxKey();
    ToxKey(const ToxKey& other);
    explicit ToxKey(const QByteArray& rawId);
    explicit ToxKey(const uint8_t* rawId, int len);

    bool operator==(const ToxKey& other) const;
    bool operator!=(const ToxKey& other) const;
    QString toString() const;
    QByteArray getKey() const;
    const uint8_t* getBytes() const;
    bool isEmpty() const;

private:
    QByteArray key;
};

#endif // TOXKEY_H
