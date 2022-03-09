/*
    Copyright © 2022 by The qTox Project Contributors

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

#include "src/core/core.h"
#include "src/core/dhtserver.h"
#include "src/model/ibootstraplistgenerator.h"

#include <QList>

#include <mutex>

class MockBootstrapListGenerator : public IBootstrapListGenerator
{
public:
    MockBootstrapListGenerator() = default;
    ~MockBootstrapListGenerator() override;

    QList<DhtServer> getBootstrapNodes() const override
    {
        const std::lock_guard<std::mutex> lock(mutex);
        return bootstrapNodes;
    }

    void setBootstrapNodes(QList<DhtServer> list)
    {
        const std::lock_guard<std::mutex> lock(mutex);
        bootstrapNodes = list;
    }

    static QList<DhtServer> makeListFromSelf(const Core& core)
    {
        auto selfDhtId = core.getSelfDhtId();
        quint16 selfDhtPort = core.getSelfUdpPort();

        DhtServer ret;
        ret.ipv4 = "localhost";
        ret.publicKey = ToxPk{selfDhtId};
        ret.statusTcp = false;
        ret.statusUdp = true;
        ret.udpPort = selfDhtPort;
        return {ret};
    }

private:
    QList<DhtServer> bootstrapNodes;
    mutable std::mutex mutex;
};
