#ifndef QTOXDNS_H
#define QTOXDNS_H

#include "src/corestructs.h"
#include <QDnsLookup>
#include <QObject>

/// Handles tox1 and tox3 DNS queries
class ToxDNS : public QObject
{
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

    static QString queryTox1(const QString& record, bool silent=true); ///< Record should look like user@domain.tld
    static QString queryTox3(const tox3_server& server, const QString& record, bool silent=true); ///< Record should look like user@domain.tld, may fallback on queryTox1

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
