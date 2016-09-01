#ifndef MOCKCORE_H
#define MOCKCORE_H

#include "src/core/core.h"

#include <QThread>

class MockCore : public Core
{
public:
    MockCore(QThread *thread, Profile &profile) : Core(thread, profile) {}

    MOCK_CONST_METHOD1(getPeerName, QString(const ToxId &id));
};


#endif // MOCKCORE_H
