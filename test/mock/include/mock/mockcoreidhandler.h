/*
    Copyright © 2020 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "src/core/icoreidhandler.h"

class MockCoreIdHandler : public ICoreIdHandler
{
public:
    MockCoreIdHandler() = default;
    virtual ~MockCoreIdHandler();
    MockCoreIdHandler(const MockCoreIdHandler&) = default;
    MockCoreIdHandler& operator=(const MockCoreIdHandler&) = default;
    MockCoreIdHandler(MockCoreIdHandler&&) = default;
    MockCoreIdHandler& operator=(MockCoreIdHandler&&) = default;

    ToxId getSelfId() const override
    {
        std::terminate();
        return ToxId();
    }

    ToxPk getSelfPublicKey() const override
    {
        static uint8_t id[ToxPk::size] = {0};
        return ToxPk(id);
    }

    QString getUsername() const override
    {
        return "me";
    }
};
