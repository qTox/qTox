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

#include "src/core/icoregroupquery.h"

#include <tox/tox.h>

/**
 * Mock 1 peer at group number 0
 */
class MockGroupQuery : public ICoreGroupQuery
{
public:
    GroupId getGroupPersistentId(uint32_t groupNumber) const override
    {
        return GroupId(0);
    }

    uint32_t getGroupNumberPeers(int groupId) const override
    {
        if (emptyGroup) {
            return 1;
        }

        return 2;
    }

    QString getGroupPeerName(int groupId, int peerId) const override
    {
        return QString("peer") + peerId;
    }

    ToxPk getGroupPeerPk(int groupId, int peerId) const override
    {
        uint8_t id[TOX_PUBLIC_KEY_SIZE] = {static_cast<uint8_t>(peerId)};
        return ToxPk(id);
    }

    QStringList getGroupPeerNames(int groupId) const override
    {
        if (emptyGroup) {
            return QStringList({QString("me")});
        }
        return QStringList({QString("me"), QString("other")});
    }

    bool getGroupAvEnabled(int groupId) const override
    {
        return false;
    }

    void setAsEmptyGroup()
    {
        emptyGroup = true;
    }

    void setAsFunctionalGroup()
    {
        emptyGroup = false;
    }

private:
    bool emptyGroup = false;
};
