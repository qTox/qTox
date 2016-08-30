#ifndef FAKESETTINGS_H
#define FAKESETTINGS_H

#include "src/persistence/settings.h"
#include "src/core/toxid.h"

#include <gmock/gmock.h>

class MockSettings : public Settings
{
public:
    MockSettings() : Settings()
    {
        settings = this;
    }

    MOCK_METHOD1(loadPersonal, void(Profile* profile));
    MOCK_CONST_METHOD1(getFriendAlias, QString(const ToxId &id));
    MOCK_METHOD2(setFriendAlias, void(const ToxId &id, const QString &alias));
};

#endif // FAKESETTINGS_H
