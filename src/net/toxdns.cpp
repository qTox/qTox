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


#include "src/net/toxdns.h"
#include "src/core/cdata.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <tox/tox.h>
#include <tox/toxdns.h>

#define TOX_HEX_ID_LENGTH 2*TOX_ADDRESS_SIZE

const ToxDNS::tox3_server ToxDNS::pinnedServers[]
{
    {"toxme.se", (uint8_t[32]){0x5D, 0x72, 0xC5, 0x17, 0xDF, 0x6A, 0xEC, 0x54, 0xF1, 0xE9, 0x77, 0xA6, 0xB6, 0xF2, 0x59, 0x14,
                0xEA, 0x4C, 0xF7, 0x27, 0x7A, 0x85, 0x02, 0x7C, 0xD9, 0xF5, 0x19, 0x6D, 0xF1, 0x7E, 0x0B, 0x13}},
    {"utox.org", (uint8_t[32]){0xD3, 0x15, 0x4F, 0x65, 0xD2, 0x8A, 0x5B, 0x41, 0xA0, 0x5D, 0x4A, 0xC7, 0xE4, 0xB3, 0x9C, 0x6B,
                  0x1C, 0x23, 0x3C, 0xC8, 0x57, 0xFB, 0x36, 0x5C, 0x56, 0xE8, 0x39, 0x27, 0x37, 0x46, 0x2A, 0x12}},
    {"sr.ht",    (uint8_t[32]){0x05, 0x08, 0x88, 0xFE, 0x0C, 0x88, 0x29, 0xAA, 0x43, 0xE0, 0xC9, 0xFD, 0x49, 0x6D, 0x6C, 0x4A,
                  0xFE, 0xEF, 0xEB, 0x6D, 0x55, 0x71, 0xF4, 0x2F, 0x30, 0xEB, 0x5F, 0x3B, 0x4C, 0xD9, 0x4F, 0xE5}},
    {"toxme.io", (uint8_t[32]){ 0x1A, 0x39, 0xE7, 0xA5, 0xD5, 0xFA, 0x9C, 0xF1, 0x55, 0xC7, 0x51, 0x57, 0x0A, 0x32, 0xE6, 0x25,
                  0x69, 0x8A, 0x60, 0xA5, 0x5F, 0x6D, 0x88, 0x02, 0x8F, 0x94, 0x9F, 0x66, 0x14, 0x4F, 0x4F, 0x25 }}
};

void ToxDNS::showWarning(const QString &message)
{
    QMessageBox warning;
    warning.setWindowTitle("Tox");
    warning.setText(message);
    warning.setIcon(QMessageBox::Warning);
    warning.exec();
}

QByteArray ToxDNS::fetchLastTextRecord(const QString& record, bool silent)
{
    QByteArray result;

    QDnsLookup dns;
    dns.setType(QDnsLookup::TXT);
    dns.setName(record);
    dns.lookup();

    int timeout;
    for (timeout = 0; timeout<30 && !dns.isFinished(); ++timeout)
    {
        qApp->processEvents();
        QThread::msleep(100);
    }
    if (timeout >= 30)
    {
        dns.abort();
        if (!silent)
            showWarning(tr("The connection timed out","The DNS gives the Tox ID associated to toxme.se addresses"));

        return result;
    }

    if (dns.error() == QDnsLookup::NotFoundError)
    {
        if (!silent)
            showWarning(tr("This address does not exist","The DNS gives the Tox ID associated to toxme.se addresses"));

        return result;
    }
    else if (dns.error() != QDnsLookup::NoError)
    {
        if (!silent)
            showWarning(tr("Error while looking up DNS","The DNS gives the Tox ID associated to toxme.se addresses"));

        return result;
    }

    const QList<QDnsTextRecord> textRecords = dns.textRecords();
    if (textRecords.isEmpty())
    {
        if (!silent)
            showWarning(tr("No text record found", "Error with the DNS"));

        return result;
    }

    const QList<QByteArray> textRecordValues = textRecords.last().values();
    if (textRecordValues.length() != 1)
    {
        if (!silent)
            showWarning(tr("Unexpected number of values in text record", "Error with the DNS"));

        return result;
    }

    result = textRecordValues.first();
    return result;
}

QString ToxDNS::queryTox1(const QString& record, bool silent)
{
    QString realRecord = record, toxId;
    realRecord.replace("@", "._tox.");
    const QString entry = fetchLastTextRecord(realRecord, silent);
    if (entry.isEmpty())
        return toxId;

    // Check toxdns protocol version
    int verx = entry.indexOf("v=");
    if (verx)
    {
        verx += 2;
        int verend = entry.indexOf(';', verx);
        if (verend)
        {
            QString ver = entry.mid(verx, verend-verx);
            if (ver != "tox1")
            {
                if (!silent)
                    showWarning(tr("The version of Tox DNS used by this server is not supported", "Error with the DNS"));

                return toxId;
            }
        }
    }

    // Get the tox id
    int idx = entry.indexOf("id=");
    if (idx < 0)
    {
        if (!silent)
            showWarning(tr("The DNS lookup does not contain any Tox ID", "Error with the DNS"));

        return toxId;
    }

    idx += 3;
    if (entry.length() < idx + static_cast<int>(TOX_HEX_ID_LENGTH))
    {
        if (!silent)
            showWarning(tr("The DNS lookup does not contain a valid Tox ID", "Error with the DNS"));

        return toxId;
    }

    toxId = entry.mid(idx, TOX_HEX_ID_LENGTH);
    if (!ToxId::isToxId(toxId))
    {
        if (!silent)
            showWarning(tr("The DNS lookup does not contain a valid Tox ID", "Error with the DNS"));

        return toxId;
    }

    return toxId;
}

QString ToxDNS::queryTox3(const tox3_server& server, const QString &record, bool silent)
{
    QByteArray nameData = record.left(record.indexOf('@')).toUtf8(), id, realRecord;
    QString entry, toxIdStr;
    int toxIdSize, idx, verx, dns_string_len;
    const int dns_string_maxlen = 128;

    void* tox_dns3 = tox_dns3_new(server.pubkey);
    if (!tox_dns3)
    {
        qWarning() << "failed to create a tox_dns3 object for "<<server.name<<", using tox1 as a fallback";
        goto fallbackOnTox1;
    }
    uint32_t request_id;
    uint8_t dns_string[dns_string_maxlen];
    dns_string_len = tox_generate_dns3_string(tox_dns3, dns_string, dns_string_maxlen, &request_id,
                             (uint8_t*)nameData.data(), nameData.size());

    if (dns_string_len < 0) // We can always fallback on tox1 if toxdns3 fails
    {
        qWarning() << "failed to generate dns3 string for "<<server.name<<", using tox1 as a fallback";
        goto fallbackOnTox1;
    }

    realRecord = '_'+QByteArray((char*)dns_string, dns_string_len)+"._tox."+server.name;
    entry = fetchLastTextRecord(realRecord, silent);
    if (entry.isEmpty())
    {
        qWarning() << "Server "<<server.name<<" returned no record, assuming the Tox ID doesn't exist";
        return toxIdStr;
    }

    // Check toxdns protocol version
    verx = entry.indexOf("v=");
    if (verx!=-1)
    {
        verx += 2;
        int verend = entry.indexOf(';', verx);
        if (verend!=-1)
        {
            QString ver = entry.mid(verx, verend-verx);
            if (ver != "tox3")
            {
                qWarning() << "Server "<<server.name<<" returned a bad version ("<<ver<<"), using tox1 as a fallback";
                goto fallbackOnTox1;
            }
        }
    }

    // Get and decrypt the tox id
    idx = entry.indexOf("id=");
    if (idx < 0)
    {
        qWarning() << "Server "<<server.name<<" returned an empty id, using tox1 as a fallback";
        goto fallbackOnTox1;
    }

    idx += 3;
    id = entry.mid(idx).toUtf8();
    uint8_t toxId[TOX_ADDRESS_SIZE];
    toxIdSize = tox_decrypt_dns3_TXT(tox_dns3, toxId, (uint8_t*)id.data(), id.size(), request_id);
    if (toxIdSize < 0) // We can always fallback on tox1 if toxdns3 fails
    {
        qWarning() << "Failed to decrypt dns3 reply for "<<server.name<<", using tox1 as a fallback";
        goto fallbackOnTox1;
    }

    tox_dns3_kill(tox_dns3);
    toxIdStr = CFriendAddress::toString(toxId);
    return toxIdStr;

    // Centralized error handling, fallback on tox1 queries
fallbackOnTox1:
    if (tox_dns3)
        tox_dns3_kill(tox_dns3);

#if TOX1_SILENT_FALLBACK
    toxIdStr = queryTox1(record, silent);
#elif TOX1_ASK_FALLBACK
    QMessageBox::StandardButton btn =
        QMessageBox::warning(nullptr, "qTox",
                             tr("It appears that qTox has to use the old protocol to access DNS record of your friend's Tox ID.\n"
                                "Unfortunately tox1 is not secure, and you are at risk of someone hijacking what is sent between you and ToxDNS service.\n"
                                "Should tox1 be used anyway?\n"
                                "If unsure, press 'No', so that request to ToxDNS service will not be made using unsecure protocol."),
                                QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if (btn == QMessageBox::Yes)
        queryTox1(record, silent);

#endif
    return toxIdStr;
}

ToxId ToxDNS::resolveToxAddress(const QString &address, bool silent)
{
    ToxId toxId;

    if (address.isEmpty())
    {
        return toxId;
    }
    else if (ToxId::isToxId(address))
    {
        toxId = ToxId(address);
        return toxId;
    }
    else
    {
        // If we're querying one of our pinned server, do a tox3 request directly
        QString servname = address.mid(address.indexOf('@')+1);
        for (const ToxDNS::tox3_server& pin : ToxDNS::pinnedServers)
        {
            if (servname == pin.name)
            {
                toxId = ToxId(queryTox3(pin, address, silent));
                return toxId;
            }
        }

        // Otherwise try tox3 if we can get a pubkey or fallback to tox1
        QByteArray pubkey = fetchLastTextRecord("_tox."+servname, true);
        if (!pubkey.isEmpty())
        {
            pubkey = QByteArray::fromHex(pubkey);

            QByteArray servnameData = servname.toUtf8();
            ToxDNS::tox3_server server;
            server.name = servnameData.data();
            server.pubkey = (uint8_t*)pubkey.data();
            toxId = ToxId(queryTox3(server, address, silent));
        }
        else
        {
#if TOX1_SILENT_FALLBACK
            toxId = ToxId::fromString(queryTox1(address, silent));
#elif TOX1_ASK_FALLBACK
            QMessageBox::StandardButton btn =
                QMessageBox::warning(nullptr, "qTox",
                                     tr("It appears that qTox has to use the old protocol to access DNS record of your friend's Tox ID.\n"
                                     "Unfortunately tox1 is not secure, and you are at risk of someone hijacking what is sent between you and ToxDNS service.\n"
                                     "Should tox1 be used anyway?\n"
                                     "If unsure, press 'No', so that request to ToxDNS service will not be made using unsecure protocol."),
                                     QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
            if (btn == QMessageBox::Ok)
                toxId = ToxId(queryTox1(address, silent));

#else
            return toxId;
#endif
        }
        return toxId;
    }
}
