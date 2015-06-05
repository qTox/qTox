/*
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
#include "src/persistence/settings.h"
#include "src/core/core.h"

#include <tox/toxencryptsave.h>

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

qint64 EncryptedDb::encryptedChunkSize = 4096;
qint64 EncryptedDb::plainChunkSize = EncryptedDb::encryptedChunkSize - TOX_PASS_ENCRYPTION_EXTRA_LENGTH;

EncryptedDb::EncryptedDb(const QString &fname, QList<QString> initList) :
    PlainDb(":memory:", initList), fileName(fname)
{
    QByteArray fileContent;
    if (pullFileContent(fileName, buffer))
    {
        qDebug() << "writing old data";
        encrFile.setFileName(fileName);
        encrFile.open(QIODevice::ReadOnly);
        fileContent = encrFile.readAll();
        chunkPosition = encrFile.size() / encryptedChunkSize;
        encrFile.close();
    }
    else
    {
        chunkPosition = 0;
    }

    encrFile.setFileName(fileName);

    if (!encrFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "can't open file:" << fileName;
    }
    else
    {
        encrFile.write(fileContent);
        encrFile.flush();
    }
}

EncryptedDb::~EncryptedDb()
{
    encrFile.close(); // Q: what if program is killed without being able to clean up?
                      // A: cleanup isn't necessary, everything handled int appendToEncrypted(..) function
}

QSqlQuery EncryptedDb::exec(const QString &query)
{
    QSqlQuery retQSqlQuery = PlainDb::exec(query);
    if (checkCmd(query))
        appendToEncrypted(query);

    return retQSqlQuery;
}

bool EncryptedDb::pullFileContent(const QString &fname, QByteArray &buf)
{
    QFile dbFile(fname);
    if (!dbFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "pullFileContent: file doesn't exist";
        buf = QByteArray();
        return false;
    }
    QByteArray fileContent;

    while (!dbFile.atEnd())
    {
        QByteArray encrChunk = dbFile.read(encryptedChunkSize);
        buf = Core::getInstance()->decryptData(encrChunk);
        if (buf.size() > 0)
        {
            fileContent += buf;
        }
        else
        {
            qWarning() << "pullFileContent: Encrypted history log is corrupted: can't decrypt, will be deleted";
            buf = QByteArray();
            return false;
        }
    }

    QList<QByteArray> splittedBA = fileContent.split('\n');
    QList<QString> sqlCmds;

    for (auto ba_line : splittedBA)
    {
        QString line = QByteArray::fromBase64(ba_line);
        sqlCmds.append(line);
    }

    PlainDb::exec("BEGIN TRANSACTION;");
    for (auto line : sqlCmds)
        QSqlQuery r = PlainDb::exec(line);

    PlainDb::exec("COMMIT TRANSACTION;");

    dbFile.close();

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
        QByteArray encr = Core::getInstance()->encryptData(filledChunk);
        if (encr.size() > 0)
        {
            encrFile.write(encr);
            encrFile.flush();
        }

        buffer = buffer.right(buffer.size() - plainChunkSize);
        chunkPosition++;
    }
    encrFile.seek(chunkPosition * encryptedChunkSize);

    QByteArray encr = Core::getInstance()->encryptData(buffer);
    if (encr.size() > 0)
        encrFile.write(encr);

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
        QByteArray buf = Core::getInstance()->decryptData(encrChunk);
        if (buf.size() == 0)
            state = false;
    }
    else
    {
        file.close();
        file.open(QIODevice::WriteOnly);
    }

    file.close();
    return state;
}

bool EncryptedDb::checkCmd(const QString &cmd)
{
    if (cmd.startsWith("INSERT", Qt::CaseInsensitive) || cmd.startsWith("UPDATE", Qt::CaseInsensitive)
            || cmd.startsWith("DELETE", Qt::CaseInsensitive))
    {
        return true;
    }

    return false;
}
