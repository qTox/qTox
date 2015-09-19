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
#include <QFile>
#include <QDebug>

unsigned char key[crypto_sign_PUBLICKEYBYTES] =
{
    0xa5, 0x80, 0xf3, 0xb7, 0xd0, 0x10, 0xc0, 0xf9, 0xd6, 0xcf, 0x48, 0x15, 0x99, 0x70, 0x92, 0x49,
    0xf6, 0xe8, 0xe5, 0xe2, 0x6c, 0x73, 0x8c, 0x48, 0x25, 0xed, 0x01, 0x72, 0xf7, 0x6c, 0x17, 0x28
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

QList<UpdateFileMeta> genUpdateDiff(QList<UpdateFileMeta> updateFlist)
{
    QList<UpdateFileMeta> diff;
    QList<UpdateFileMeta> localFlist = parseFlist(getLocalFlist());

    for (UpdateFileMeta file : updateFlist)
        if (!localFlist.contains(file))
            diff += file;

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
