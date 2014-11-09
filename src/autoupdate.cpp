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


#include "src/autoupdate.h"
#include "src/misc/serialize.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCoreApplication>

#ifdef _WIN32
const QString AutoUpdater::platform = "win32";
#else
const QString AutoUpdater::platform = "win32"; ///TODO: FIXME: undefine, we want an empty qstring
#endif
const QString AutoUpdater::updateServer = "http://127.0.0.1";
const QString AutoUpdater::checkURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/version";
const QString AutoUpdater::flistURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/flist";
const QString AutoUpdater::filesURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/files/";
unsigned char AutoUpdater::key[crypto_sign_PUBLICKEYBYTES] =
{
    0xa5, 0x80, 0xf3, 0xb7, 0xd0, 0x10, 0xc0, 0xf9, 0xd6, 0xcf, 0x48, 0x15, 0x99, 0x70, 0x92, 0x49,
    0xf6, 0xe8, 0xe5, 0xe2, 0x6c, 0x73, 0x8c, 0x48, 0x25, 0xed, 0x01, 0x72, 0xf7, 0x6c, 0x17, 0x28
};

bool AutoUpdater::isUpdateAvailable()
{
    QString newVersion = getUpdateVersion();
    if (newVersion.isEmpty() || newVersion == GIT_VERSION)
        return false;
    else
        return true;
}

QString AutoUpdater::getUpdateVersion()
{
    QString version;

    // Updates only for supported platforms
    if (platform.isEmpty())
        return version;

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(checkURI)));
    while (!reply->isFinished())
        qApp->processEvents();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "AutoUpdater: getUpdateVersion: network error: "<<reply->errorString();
        reply->deleteLater();
        manager->deleteLater();
        return version;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    manager->deleteLater();
    if (data.size() < (int)(1+crypto_sign_BYTES))
        return version;

    // Check updater protocol version
    if ((int)data[0] != '1')
    {
        qWarning() << "AutoUpdater: getUpdateVersion: Bad version "<<(uint8_t)data[0];
        return version;
    }

    // Check the signature
    QByteArray sigData = data.mid(1, crypto_sign_BYTES);
    unsigned char* sig = (unsigned char*)sigData.data();
    QByteArray msgData = data.mid(1+crypto_sign_BYTES);
    unsigned char* msg = (unsigned char*)msgData.data();

    if (crypto_sign_verify_detached(sig, msg, msgData.size(), key) != 0)
    {
        qCritical() << "AutoUpdater: getUpdateVersion: RECEIVED FORGED VERSION FILE FROM "<<updateServer;
        return version;
    }

    version = msgData;

    return version;
}

QList<AutoUpdater::UpdateFileMeta> AutoUpdater::genUpdateDiff()
{
    QList<UpdateFileMeta> diff;

    // Updates only for supported platforms
    if (platform.isEmpty())
        return diff;

    QList<UpdateFileMeta> newFlist = getUpdateFlist();

    return diff;
}

QList<AutoUpdater::UpdateFileMeta> AutoUpdater::parseflist(QByteArray flistData)
{
    QList<UpdateFileMeta> flist;

    if (flistData.isEmpty())
    {
        qWarning() << "AutoUpdater::parseflist: Empty data";
        return flist;
    }

    // Check version
    if (flistData[0] != '1')
    {
        qWarning() << "AutoUpdater: parseflist: Bad version "<<(uint8_t)flistData[0];
        return flist;
    }
    flistData = flistData.mid(1);

    // Check signature
    if (flistData.size() < (int)(crypto_sign_BYTES))
    {
        qWarning() << "AutoUpdater::parseflist: Truncated data";
        return flist;
    }
    else
    {
        QByteArray msgData = flistData.mid(crypto_sign_BYTES);
        unsigned char* msg = (unsigned char*)msgData.data();
        if (crypto_sign_verify_detached((unsigned char*)flistData.data(), msg, msgData.size(), key) != 0)
        {
            qCritical() << "AutoUpdater: parseflist: FORGED FLIST FILE";
            return flist;
        }
        flistData = flistData.mid(crypto_sign_BYTES);
    }

    // Parse. We assume no errors handling needed since the signature is valid.
    while (!flistData.isEmpty())
    {
        qDebug() << "Got "<<flistData.size()<<" bytes of data left";

        UpdateFileMeta newFile;

        memcpy(newFile.sig, flistData.data(), crypto_sign_BYTES);
        flistData = flistData.mid(crypto_sign_BYTES);

        newFile.id = dataToString(flistData);
        flistData = flistData.mid(newFile.id.size() + getVUint32Size(flistData));

        newFile.installpath = dataToString(flistData);
        flistData = flistData.mid(newFile.installpath.size() + getVUint32Size(flistData));

        newFile.size = dataToUint64(flistData);
        flistData = flistData.mid(8);

        qDebug() << "AutoUpdater::parseflist: New file:";
        qDebug() << "- Id: "<<newFile.id;
        qDebug() << "- Install path: "<<newFile.installpath;
        qDebug() << "- Size: "<<newFile.size<<" bytes";
        qDebug() << "- Signature: "<<QByteArray((char*)newFile.sig, crypto_sign_BYTES).toHex();

        flist += newFile;
    }
    qDebug() << "AutoUpdater::parseflist: Done parsing flist";

    return flist;
}

QList<AutoUpdater::UpdateFileMeta> AutoUpdater::getUpdateFlist()
{
    QList<UpdateFileMeta> flist;

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(flistURI)));
    while (!reply->isFinished())
        qApp->processEvents();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "AutoUpdater: getUpdateFlist: network error: "<<reply->errorString();
        reply->deleteLater();
        manager->deleteLater();
        return flist;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    manager->deleteLater();

    flist = parseflist(data);
    return flist;
}
