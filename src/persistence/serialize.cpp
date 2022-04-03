/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "src/persistence/serialize.h"

/**
 * @file serialize.cpp
 * Most of functions in this file are unsafe unless otherwise specified.
 * @warning Do not use them on untrusted data (e.g. check a signature first).
 */

QString dataToString(QByteArray data)
{
    char num3;
    int strlen = 0;
    int num2 = 0;
    int i = 0;
    do {
        num3 = data[i++];
        strlen |= (num3 & 0x7f) << num2;
        num2 += 7;
    } while ((num3 & 0x80) != 0);

    if (strlen <= 0)
        return QString();

    // Remove the strlen
    data.remove(0, i);
    data.truncate(strlen);

    return QString::fromUtf8(data);
}

uint64_t dataToUint64(const QByteArray& data)
{
    return static_cast<uint64_t>(data[0]) | (static_cast<uint64_t>(data[1]) << 8)
           | (static_cast<uint64_t>(data[2]) << 16) | (static_cast<uint64_t>(data[3]) << 24)
           | (static_cast<uint64_t>(data[4]) << 32) | (static_cast<uint64_t>(data[5]) << 40)
           | (static_cast<uint64_t>(data[6]) << 48) | (static_cast<uint64_t>(data[7]) << 56);
}

int dataToVInt(const QByteArray& data)
{
    char num3;
    int num = 0;
    int num2 = 0;
    int i = 0;
    do {
        num3 = data[i++];
        num |= static_cast<int>(num3 & 0x7f) << num2;
        num2 += 7;
    } while ((num3 & 0x80) != 0);
    return num;
}

size_t dataToVUint(const QByteArray& data)
{
    char num3;
    size_t num = 0;
    int num2 = 0;
    int i = 0;
    do {
        num3 = data[i++];
        num |= static_cast<size_t>(num3 & 0x7f) << num2;
        num2 += 7;
    } while ((num3 & 0x80) != 0);
    return num;
}

unsigned getVUint32Size(QByteArray data)
{
    unsigned lensize = 0;

    char num3;
    do {
        num3 = data[lensize];
        ++lensize;
    } while ((num3 & 0x80) != 0);

    return lensize;
}

QByteArray vintToData(int num)
{
    QByteArray data(sizeof(int), 0);
    // Write the size in a Uint of variable lenght (8-32 bits)
    int i = 0;
    while (num >= 0x80) {
        data[i] = static_cast<char>(num | 0x80);
        ++i;
        num = num >> 7;
    }
    data[i] = static_cast<char>(num);
    data.resize(i + 1);
    return data;
}

QByteArray vuintToData(size_t num)
{
    QByteArray data(sizeof(size_t), 0);
    // Write the size in a Uint of variable lenght (8-32 bits)
    int i = 0;
    while (num >= 0x80) {
        data[i] = static_cast<char>(num | 0x80);
        ++i;
        num = num >> 7;
    }
    data[i] = static_cast<char>(num);
    data.resize(i + 1);
    return data;
}
