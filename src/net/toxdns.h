/*
    Copyright © 2014-2015 by The qTox Project

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


#ifndef QTOXDNS_H
#define QTOXDNS_H

#include "src/core/corestructs.h"
#include "src/core/toxid.h"
#include <QDnsLookup>
#include <QObject>

class ToxDNS : public QObject
{
    Q_OBJECT

public:
    struct tox3_server
    {
        tox3_server()=default;
        tox3_server(const char* _name, uint8_t _pk[32]):name{_name},pubkey{_pk}{}

        const char* name;
        uint8_t* pubkey;
    };

public:
    static ToxId resolveToxAddress(const QString& address, bool silent=true);
    static QString queryTox3(const tox3_server& server, const QString& record, bool silent=true);

protected:
    static void showWarning(const QString& message);
    ToxDNS()=default;

private:
    static QByteArray fetchLastTextRecord(const QString& record, bool silent=true);

public:
    static const tox3_server pinnedServers[4];
};

#endif // QTOXDNS_H
