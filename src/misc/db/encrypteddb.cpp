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

qint64 EncryptedDb::plainChunkSize = 4096;
qint64 EncryptedDb::encryptedChunkSize = EncryptedDb::plainChunkSize + tox_pass_encryption_extra_length();

EncryptedDb::EncryptedDb(const QString &fname, QList<QString> initList) :
    PlainDb(":memory:", initList), encrFile(fname)
{
    QByteArray fileContent;
    if (pullFileContent())
    {
        chunkPosition = encrFile.size() / encryptedChunkSize;

        encrFile.seek(0);
        fileContent = encrFile.readAll();
    } else {
        qWarning() << "corrupted history log file will be wiped!";
        chunkPosition = 0;
    }

    encrFile.close();
    encrFile.open(QIODevice::WriteOnly);
    encrFile.write(fileContent);
    encrFile.flush();
}

EncryptedDb::~EncryptedDb()
{
    encrFile.close(); // Q: what if program is killed without being able to clean up?
                      // A: cleanup isn't necessary, everything handled int appendToEncrypted(..) function
}

QSqlQuery EncryptedDb::exec(const QString &query)
{
    QSqlQuery retQSqlQuery = PlainDb::exec(query);
    if (query.startsWith("INSERT", Qt::CaseInsensitive))
        appendToEncrypted(query);

    return retQSqlQuery;
}

bool EncryptedDb::pullFileContent()
{
    qDebug() << "EncryptedDb::pullFileContent()";
    encrFile.open(QIODevice::ReadOnly);
    QByteArray fileContent;

    while (!encrFile.atEnd())
    {
        QByteArray encrChunk = encrFile.read(encryptedChunkSize);
        qDebug() << "got chunk:" << encrChunk.size();
        buffer = Core::getInstance()->decryptData(encrChunk, Core::ptHistory);
        if (buffer.size() > 0)
        {
            fileContent += buffer;
        } else {
            qWarning() << "Encrypted history log is corrupted: can't decrypt";
            buffer = QByteArray();
            return false;
        }
    }

    QList<QByteArray> splittedBA = fileContent.split('\n');
    QList<QString> sqlCmds;

    for (auto ba_line : splittedBA)
    {
        QString line = QByteArray::fromBase64(ba_line);
        if (line.size() == 0)
            continue;

        bool isGoodLine = false;
        if (line.startsWith("CREATE", Qt::CaseInsensitive) || line.startsWith("INSERT", Qt::CaseInsensitive))
        {
            if (line.endsWith(");"))
            {
                sqlCmds.append(line);
                isGoodLine = true;
            }
        }

        if (!isGoodLine)
        {
            qWarning() << "Encrypted history log is corrupted: errors in content";
            buffer = QByteArray();
            return false;
        }
    }

    for (auto line : sqlCmds)
    {
        QSqlQuery r = PlainDb::exec(line);
    }

    return true;
}

void EncryptedDb::appendToEncrypted(const QString &sql)
{
    QByteArray b64Str;
    b64Str.append(sql);
    b64Str = b64Str.toBase64();  // much easier to parse strings like this from file

    buffer += b64Str + "\n";

    while (buffer.size() > plainChunkSize)
    {
        QByteArray filledChunk = buffer.left(plainChunkSize);
        encrFile.seek(chunkPosition * encryptedChunkSize);
        QByteArray encr = Core::getInstance()->encryptData(filledChunk, Core::ptHistory);
        if (encr.size() > 0)
        {
            encrFile.write(encr);
        }

        buffer = buffer.right(buffer.size() - plainChunkSize);
        chunkPosition++;
    }
    encrFile.seek(chunkPosition * encryptedChunkSize);

    QByteArray encr = Core::getInstance()->encryptData(buffer, Core::ptHistory);
    if (encr.size() > 0)
    {
        encrFile.write(encr);
    }
    encrFile.flush();
}

bool EncryptedDb::check(const QString &fname)
{
    QFile file(fname);
    file.open(QIODevice::ReadOnly);
    bool state = true;

    if (file.size() > 0)
    {
        QByteArray encrChunk = file.read(encryptedChunkSize);
        QByteArray buf = Core::getInstance()->decryptData(encrChunk, Core::ptHistory);
        if (buf.size() == 0)
        {
            state = false;
        }
    } else {
        file.close();
        file.open(QIODevice::WriteOnly);
    }

    file.close();
    return state;
}
