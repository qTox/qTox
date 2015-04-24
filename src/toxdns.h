/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/


#ifndef QTOXDNS_H
#define QTOXDNS_H

#include "src/core/corestructs.h"
#include <QDnsLookup>
#include <QObject>

/// Tox1 is not encrypted, it's unsafe
#define TOX1_SILENT_FALLBACK 0

/// That said if the user insists ...
#define TOX1_ASK_FALLBACK 1

/// Handles tox1 and tox3 DNS queries
class ToxDNS : public QObject
{
    Q_OBJECT

public:
    struct tox3_server ///< Represents a tox3 server
    {
        tox3_server()=default;
        tox3_server(const char* _name, uint8_t _pk[32]):name{_name},pubkey{_pk}{}

        const char* name; ///< Hostname of the server, e.g. toxme.se
        uint8_t* pubkey; ///< Public key of the tox3 server, usually 256bit long
    };

public:
    /// Tries to map a text string to a ToxID struct, will query Tox DNS records if necessary
    static ToxID resolveToxAddress(const QString& address, bool silent=true);

    static QString queryTox1(const QString& record, bool silent=true); ///< Record should look like user@domain.tld. Do *NOT* use tox1 without a good reason, it's unsafe.
    static QString queryTox3(const tox3_server& server, const QString& record, bool silent=true); ///< Record should look like user@domain.tld, will *NOT* fallback on queryTox1 anymore

protected:
    static void showWarning(const QString& message);
    ToxDNS()=default;

private:
    /// Try to fetch the first entry of the given TXT record
    /// Returns an empty object on failure. May block for up to ~3s
    /// May display message boxes on error if silent if false
    static QByteArray fetchLastTextRecord(const QString& record, bool silent=true);

public:
    static const tox3_server pinnedServers[2];
};

#endif // QTOXDNS_H
