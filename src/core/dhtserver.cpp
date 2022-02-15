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

#include "dhtserver.h"

/**
 * @brief   Compare equal operator
 * @param   other   the compared instance
 * @return  true, if equal; false otherwise
 */
bool DhtServer::operator==(const DhtServer& other) const
{
    return this == &other ||
        (statusUdp == other.statusUdp
        && statusTcp == other.statusTcp
        && ipv4 == other.ipv4
        && ipv6 == other.ipv6
        && maintainer == other.maintainer
        && publicKey == other.publicKey
        && udpPort == other.udpPort
        && tcpPorts == other.tcpPorts);
}

/**
 * @brief   Compare not equal operator
 * @param   other   the compared instance
 * @return  true, if not equal; false otherwise
 */
bool DhtServer::operator!=(const DhtServer& other) const
{
    return !(*this == other);
}
