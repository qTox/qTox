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
