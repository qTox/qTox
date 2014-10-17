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

#ifndef ENCRYPTEDDB_H
#define ENCRYPTEDDB_H

#include "plaindb.h"

#include <QList>
#include <QFile>

class EncryptedDb : public PlainDb
{
public:
    EncryptedDb(const QString& fname);
    virtual ~EncryptedDb();

    virtual QSqlQuery exec(const QString &query);
    virtual bool save();

private:
    bool pullFileContent();
    void appendToEncrypted(const QString &sql);

    QFile encrFile;

    qint64 plainChunkSize;
    qint64 encryptedChunkSize;
    qint64 chunkPosition;
    QByteArray buffer;
};

#endif // ENCRYPTEDDB_H
