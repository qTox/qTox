#ifndef TOXPK_H
#define TOXPK_H

#include "src/core/contactid.h"
#include <QByteArray>
#include <cstdint>

class ToxPk : public ContactId
{
public:
    ToxPk();
    ToxPk(const ToxPk& other);
    explicit ToxPk(const QByteArray& rawId);
    explicit ToxPk(const uint8_t* rawId);
    int getSize() const override;
    Type type() const override final;
};

#endif // TOXPK_H
