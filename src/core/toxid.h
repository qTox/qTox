#ifndef TOXID_H
#define TOXID_H

#include <QString>

class ToxId
{
public:
    ToxId();
    ToxId(const ToxId& other);
    ToxId(const QString& id);

    bool operator==(const ToxId& other) const;
    bool operator!=(const ToxId& other) const;
    QString toString() const;
    void clear();

    static bool isToxId(const QString& id);

public:
    QString publicKey;
    QString noSpam;
    QString checkSum;
};

#endif // TOXID_H
