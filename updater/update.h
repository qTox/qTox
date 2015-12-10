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


#ifndef UPDATE_H
#define UPDATE_H

#include <QByteArray>
#include <QString>
#include <sodium.h>

class Widget;

struct UpdateFileMeta
{
    unsigned char sig[crypto_sign_BYTES]; ///< Signature of the file (ed25519)
    QString id; ///< Unique id of the file
    QString installpath; ///< Local path including the file name. May be relative to qtox-updater or absolute
    uint64_t size; ///< Size in bytes of the file

    bool operator==(const UpdateFileMeta& other)
    {
        return (size == other.size
                && id == other.id && installpath == other.installpath
                && memcmp(sig, other.sig, crypto_sign_BYTES) == 0);
    }
};

struct UpdateFile
{
    UpdateFileMeta metadata;
    QByteArray data;
};

/// Gets the local flist. Returns an empty array on error
QByteArray getLocalFlist();
/// Parses and validates a flist file. Returns an empty list on error
QList<UpdateFileMeta> parseFlist(QByteArray flistData);
/// Generates a list of files we need to update
QList<UpdateFileMeta> genUpdateDiff(QList<UpdateFileMeta> updateFlist, Widget *w);

extern unsigned char key[crypto_sign_PUBLICKEYBYTES];

#endif // UPDATE_H
