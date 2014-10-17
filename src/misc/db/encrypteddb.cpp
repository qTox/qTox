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

#include <tox/toxencryptsave.h>

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

EncryptedDb::EncryptedDb(const QString &fname, const QString &key) :
    PlainDb(":memory:"), encrFile(fname)
{
    encrkey = new u_int8_t[tox_pass_key_length()];
    QByteArray key_ba;
    key_ba.append(key);
//    tox_derive_key_from_pass(reinterpret_cast<uint8_t*>(key_ba.data()), key_ba.size(), encrkey);
    passwd = "test";

    qDebug() << QByteArray::fromRawData(reinterpret_cast<char *>(encrkey), tox_pass_key_length()).toBase64();

    plainChunkSize = 1024;
    encryptedChunkSize = plainChunkSize + tox_pass_encryption_extra_length();

    QByteArray fileContent;
    if (pullFileContent())
    {
        chunkPosition = encrFile.size() / encryptedChunkSize;

        encrFile.seek(0);
        fileContent = encrFile.readAll();

        /*
        if (encrFile.size() > 0)
        {
            encrFile.copy(fname + "~");
        }
        */
    } else {
        qWarning() << "corrupted history log file will be wiped!";
        chunkPosition = 0;
    }

    encrFile.close();
    encrFile.open(QIODevice::WriteOnly);
    encrFile.write(fileContent);
}

EncryptedDb::~EncryptedDb()
{
    encrFile.close();
    delete encrkey;
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

bool EncryptedDb::pullFileContent()
{
    encrFile.open(QIODevice::ReadOnly);
    QByteArray fileContent;

    while (!encrFile.atEnd())
    {
        QByteArray encrChunk = encrFile.read(encryptedChunkSize);
        buffer = decrypt(encrChunk);
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
        qDebug() << r.lastError();
    }

    return true;
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

        QByteArray encr = encrypt(filledChunk);
        if (encr.size() > 0)
        {
            encrFile.write(encr);
        }
        buffer = buffer.right(buffer.size() - plainChunkSize);
        chunkPosition++;
    }
    encrFile.seek(chunkPosition * encryptedChunkSize);

    QByteArray encr = encrypt(buffer);
    if (encr.size() > 0)
    {
        encrFile.write(encrypt(buffer));
        encrFile.flush();
    }

    qDebug() << sql;
}

QByteArray EncryptedDb::encrypt(QByteArray data)
{
    int encrSize = data.size() + tox_pass_encryption_extra_length();
    int plainSize = data.size();

    uint8_t *out = new u_int8_t[encrSize];
    //int state = tox_pass_key_encrypt(reinterpret_cast<uint8_t*>(data.data()), plainSize, encrkey, out);
    int state = tox_pass_encrypt(reinterpret_cast<uint8_t*>(data.data()), plainSize,
                                 reinterpret_cast<uint8_t*>(passwd.data()), passwd.size(), out);

    if (state == -1)
    {
        qWarning() << "encryption failed!";
        delete out;
        return QByteArray();
    }

    QByteArray ret = QByteArray::fromRawData(reinterpret_cast<const char*>(out), encrSize);
    return ret;
}

QByteArray EncryptedDb::decrypt(QByteArray data)
{
    int encrSize = data.size();
    int plainSize = data.size() - tox_pass_encryption_extra_length();

    uint8_t *out = new u_int8_t[plainSize];
    //int decrSize = tox_pass_key_decrypt(reinterpret_cast<uint8_t*>(data.data()), encrSize, encrkey, out);
    int decrSize = tox_pass_decrypt(reinterpret_cast<uint8_t*>(data.data()), encrSize,
                                    reinterpret_cast<uint8_t*>(passwd.data()), passwd.size(), out);

    if (decrSize != plainSize)
    {
        qWarning() << "decryption failed!";
        delete out;
        return QByteArray();
    }

    QByteArray ret = QByteArray::fromRawData(reinterpret_cast<const char*>(out), plainSize);
    return ret;
}
