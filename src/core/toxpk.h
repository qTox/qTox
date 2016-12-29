#ifndef TOXKEY_H
#define TOXKEY_H

#include <cstdint>
#include <QByteArray>
#include <QByteArray>

class ToxPk
{
public:
    ToxPk();
    ToxPk(const ToxPk& other);
    explicit ToxPk(const QByteArray& rawId);
    explicit ToxPk(const uint8_t* rawId, int len);

    bool operator==(const ToxPk& other) const;
    bool operator!=(const ToxPk& other) const;
    QString toString() const;
    const uint8_t* getBytes() const;
    bool isEmpty() const;

private:
    QByteArray key;
};

#endif // TOXKEY_H
