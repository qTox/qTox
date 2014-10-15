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

#include "encrypteddb.h"
#include "src/misc/settings.h"

#include <QDir>
#include <QSqlQuery>

EncryptedDb::EncryptedDb(const QString &fname, const QString &key) :
    key(key), encrFile(fname), PlainDb(":memory:")
{
    QList<QString> sqlCommands = decryptFile();
    for (const QString &cmd : sqlCommands)
    {
        PlainDb::exec(true, cmd);
    }
}

EncryptedDb::~EncryptedDb()
{
    // save to file if necessary
}

QSqlQuery EncryptedDb::exec(bool keep, const QString &query)
{
    QSqlQuery ret = PlainDb::exec(keep, query);
    if (keep)
        appendToEncrypted(query);

    return ret;
}

bool EncryptedDb::save()
{
    return true;
}

QList<QString> EncryptedDb::decryptFile()
{
    return QList<QString>();
}

void EncryptedDb::appendToEncrypted(const QString &sql)
{
    Q_UNUSED(sql)
}
