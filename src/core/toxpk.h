#ifndef ToxPk_H
#define ToxPk_H

#include <cstdint>
#include <QByteArray>
#include <QString>

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
    QByteArray getKey() const;
    const uint8_t* getBytes() const;
    bool isEmpty() const;

private:
    QByteArray key;
};

#endif // ToxPk_H
