#pragma once

#include "src/core/icoreidhandler.h"

#include <tox/tox.h>

class MockCoreIdHandler : public ICoreIdHandler
{
public:
    ToxId getSelfId() const override
    {
        std::terminate();
        return ToxId();
    }

    ToxPk getSelfPublicKey() const override
    {
        static uint8_t id[TOX_PUBLIC_KEY_SIZE] = {0};
        return ToxPk(id);
    }

    QString getUsername() const override
    {
        return "me";
    }
};
