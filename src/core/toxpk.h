#ifndef TOXPK_H
#define TOXPK_H

#include <cstdint>
#include <QByteArray>
#include <QString>

class ToxPk
{
public:
    ToxPk();
    ToxPk(const ToxPk& other);
    explicit ToxPk(const QByteArray& rawId);
    explicit ToxPk(const uint8_t* rawId);

    bool operator==(const ToxPk& other) const;
    bool operator!=(const ToxPk& other) const;
    QString toString() const;
    QByteArray getKey() const;
    const uint8_t* getBytes() const;
    bool isEmpty() const;

private:
    QByteArray key;
};

#endif // TOXPK_H
