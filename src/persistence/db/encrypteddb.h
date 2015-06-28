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
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ENCRYPTEDDB_H
#define ENCRYPTEDDB_H

#include "plaindb.h"
#include <tox/toxencryptsave.h>

#include <QList>
#include <QFile>

class EncryptedDb : public PlainDb
{
public:
    EncryptedDb(const QString& fname, QList<QString> initList);
    virtual ~EncryptedDb();

    virtual QSqlQuery exec(const QString &query);
    static bool check(const TOX_PASS_KEY &passkey, const QString &fname);

private:
    bool pullFileContent(const QString& fname, QByteArray &buf);
    void appendToEncrypted(const QString &sql);
    bool checkCmd(const QString &cmd);

    QFile encrFile;
    QString fileName;

    static qint64 plainChunkSize;
    static qint64 encryptedChunkSize;

    static TOX_PASS_KEY decryptionKey; ///< When importing, the decryption key may not be the same as the profile key

    qint64 chunkPosition;
    QByteArray buffer;
};

#endif // ENCRYPTEDDB_H
