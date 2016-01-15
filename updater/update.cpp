/*
    Copyright © 2014 by The qTox Project

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
    along with qTox.  If not, see <http://www.gnu.org/licenses/>
*/


#include "update.h"
#include "serialize.h"
#include "widget.h"
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>

unsigned char key[crypto_sign_PUBLICKEYBYTES] =
{
    0x20, 0x89, 0x39, 0xaa, 0x9a, 0xe8, 0xb5, 0x21, 0x0e, 0xac, 0x02, 0xa9, 0xc4, 0x92, 0xd9, 0xa2,
    0x17, 0x83, 0xbd, 0x78, 0x0a, 0xda, 0x33, 0xcd, 0xa5, 0xc6, 0x44, 0xc7, 0xfc, 0xed, 0x00, 0x13
};

QByteArray getLocalFlist()
{
    QByteArray flist;

    QFile flistFile("flist");
    if (!flistFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "getLocalFlist: Can't open local flist";
        return flist;
    }

    flist = flistFile.readAll();
    flistFile.close();

    return flist;
}

bool isUpToDate(UpdateFileMeta fileMeta)
{
    QString appDir = qApp->applicationDirPath();
    QFile file(appDir+QDir::separator()+fileMeta.installpath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    // If the data we have is corrupted or old, mark it for update
    QByteArray data = file.readAll();
    if (crypto_sign_verify_detached(fileMeta.sig, (unsigned char*)data.data(), data.size(), key) != 0)
        return false;

    return true;
}

QList<UpdateFileMeta> genUpdateDiff(QList<UpdateFileMeta> updateFlist, Widget* w)
{
    QList<UpdateFileMeta> diff;

    float progressDiff = 45;
    float progress = 5;

    for (UpdateFileMeta file : updateFlist)
    {
        if (!isUpToDate(file))
            diff += file;
        progress += progressDiff / updateFlist.size();
        w->setProgress(progress);
    }

    return diff;
}

QList<UpdateFileMeta> parseFlist(QByteArray flistData)
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
        UpdateFileMeta newFile;

        memcpy(newFile.sig, flistData.data(), crypto_sign_BYTES);
        flistData = flistData.mid(crypto_sign_BYTES);

        newFile.id = dataToString(flistData);
        flistData = flistData.mid(newFile.id.size() + getVUint32Size(flistData));

        newFile.installpath = dataToString(flistData);
        flistData = flistData.mid(newFile.installpath.size() + getVUint32Size(flistData));

        newFile.size = dataToUint64(flistData);
        flistData = flistData.mid(8);

        flist += newFile;
    }

    return flist;
}
