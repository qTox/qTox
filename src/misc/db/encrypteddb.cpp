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
#include "src/core.h"

#include <tox/toxencryptsave.h>

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

EncryptedDb::EncryptedDb(const QString &fname) :
    PlainDb(":memory:"), encrFile(fname)
{
    plainChunkSize = 1024;
    encryptedChunkSize = plainChunkSize + tox_pass_encryption_extra_length();

    encrFile.open(QIODevice::ReadOnly);

    QList<QString> sqlCommands = decryptFile();
    for (const QString &cmd : sqlCommands)
    {
        // check line here
        QSqlQuery r = PlainDb::exec(cmd);
        qDebug() << r.lastError();
    }

    chunkPosition = encrFile.size() / encryptedChunkSize;
//    encrFile.seek(chunkPosition * encryptedChunkSize);
//    buffer = encrFile.read(encrFile.size() % encryptedChunkSize);

    encrFile.seek(0);
    QByteArray fileContent = encrFile.readAll();
    encrFile.close();
    encrFile.open(QIODevice::WriteOnly);
    encrFile.write(fileContent);
}

EncryptedDb::~EncryptedDb()
{
    encrFile.close(); // what if program is killed without being able to clean up?
}

QSqlQuery EncryptedDb::exec(const QString &query)
{
    QSqlQuery retQSqlQuery = PlainDb::exec(query);
    if (query.startsWith("INSERT", Qt::CaseInsensitive) || query.startsWith("CREATE", Qt::CaseInsensitive))
        appendToEncrypted(query);

    return retQSqlQuery;
}

bool EncryptedDb::save()
{
    return true;
}

QList<QString> EncryptedDb::decryptFile()
{
    QByteArray fileContent;

    while (!encrFile.atEnd())
    {
        QByteArray encrChunk = encrFile.read(encryptedChunkSize);
        buffer = Core::getInstance()->decryptData(encrChunk);
        fileContent += buffer;
    }

    QList<QByteArray> splittedBA = fileContent.split('\n');
    QList<QString> res;
    for (auto ba_line : splittedBA)
    {
        QString line = QByteArray::fromBase64(ba_line);
        //check line correctness here
        res.append(line);
//        res.append(ba_line);
    }

    return res;
}

void EncryptedDb::appendToEncrypted(const QString &sql)
{
    QByteArray b64Str;
    b64Str.append(sql);
    b64Str = b64Str.toBase64();

    buffer += b64Str + "\n";

    while (buffer.size() > plainChunkSize)
    {
        QByteArray filledChunk = buffer.left(plainChunkSize);
        encrFile.seek(chunkPosition * encryptedChunkSize);
        encrFile.write(Core::getInstance()->encryptData(filledChunk));
        buffer = buffer.right(buffer.size() - plainChunkSize);
        chunkPosition++;
    }
    encrFile.seek(chunkPosition * encryptedChunkSize);

    encrFile.write(Core::getInstance()->encryptData(buffer));
    encrFile.flush();

    qDebug() << sql;
}
