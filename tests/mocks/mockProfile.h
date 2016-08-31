#ifndef MOCKPROFILE_H
#define MOCKPROFILE_H

#include "src/persistence/profile.h"

class MockProfile : public Profile
{
public:
    MockProfile(QString name, QString pass, bool newProfile)
        : Profile(name, pass, newProfile) {}

    void setCore(Core *core)
    {
        this->core = core;
    }

    ~MockProfile()
    {
        // For a profile not to try to save itself
        isRemoved = true;
    }
};

#endif // MOCKPROFILE_H
