/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#include "toxpk.h"

#include <QString>
#include <vector>

struct DhtServer
{
    bool statusUdp;
    bool statusTcp;
    QString ipv4;
    QString ipv6;
    QString maintainer;
    ToxPk publicKey;
    quint16 udpPort;
    std::vector<uint16_t> tcpPorts;

    bool operator==(const DhtServer& other) const;
    bool operator!=(const DhtServer& other) const;
};
