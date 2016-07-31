/*
    Copyright © 2014-2015 by The qTox Project

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
@file serialize.cpp
Most of functions in this file are unsafe unless otherwise specified.
@warning Do not use them on untrusted data (e.g. check a signature first).
*/

QByteArray doubleToData(double num)
{
    union
    {
        char tab[8];
        double n;
    } castUnion;
    //char n[8];
    //*((double*) n) = num;

    castUnion.n=num;
    return QByteArray(castUnion.tab,8);
}

QByteArray floatToData(float num)
{
    union
    {
        char tab[4];
        float n;
    } castUnion;

    castUnion.n=num;
    return QByteArray(castUnion.tab,4);
}

float dataToFloat(QByteArray data)
{
    union
    {
        char tab[4];
        float n;
    } castUnion;

    castUnion.tab[0]=data.data()[0];
    castUnion.tab[1]=data.data()[1];
    castUnion.tab[2]=data.data()[2];
    castUnion.tab[3]=data.data()[3];
    return castUnion.n;
}

// Converts a string into PNet string data
QByteArray stringToData(const QString& str)
{
    QByteArray data(4,0);
    // Write the size in a Uint of variable lenght (8-32 bits)
    int i=0;
    int num1 = str.toUtf8().size();
    while (num1 >= 0x80)
    {
        data[i] = static_cast<char>(num1 | 0x80);
        i++;
        num1 = num1 >> 7;
    }
    data[i] = static_cast<char>(num1);
    data.resize(i+1);
    data+=str.toUtf8();
    return data;
}

QString dataToString(QByteArray data)
{
    char num3;
    int strlen = 0;
    int num2 = 0;
    int i=0;
    do
    {
        num3 = data[i++];
        strlen |= (num3 & 0x7f) << num2;
        num2 += 7;
    } while ((num3 & 0x80) != 0);

    if (strlen <= 0)
        return QString();

    // Remove the strlen
    data.remove(0, i - 1);
    data.truncate(strlen);

    return QString(data);
}

float dataToRangedSingle(float min, float max, int numberOfBits, QByteArray data)
{
    uint endvalue=0;
    uint value=0;
    if (numberOfBits <= 8)
    {
        endvalue = static_cast<uchar>(data[0]);
        goto done;
    }
    value = static_cast<uchar>(data[0]);
    numberOfBits -= 8;
    if (numberOfBits <= 8)
    {
        endvalue = value | (static_cast<uint>(data[1]) << 8);
        goto done;
    }
    value |= static_cast<uint>(data[1]) << 8;
    numberOfBits -= 8;
    if (numberOfBits <= 8)
    {
        uint num2 = static_cast<uint>(data[2]) << 0x10;
        endvalue = (value | num2);
        goto done;
    }
    value |= static_cast<uint>(data[2]) << 0x10;
    numberOfBits -= 8;
    endvalue = value | (static_cast<uint>(data[3]) << 0x18);
    goto done;

    done:

    float num = max - min;
    int num2 = (static_cast<int>(1) << numberOfBits) - 1;
    float num3 = endvalue;
    float num4 = num3 / num2;
    return (min + (num4 * num));
}

QByteArray rangedSingleToData(float value, float min, float max, int numberOfBits)
{
    QByteArray data;
    float num = max - min;
    float num2 = (value - min) / num;
    int num3 = (static_cast<int>(1) << numberOfBits) - 1;
    uint source = static_cast<uint>(num3 * num2);

    if (numberOfBits <= 8)
    {
        data += static_cast<char>(source);
        return data;
    }
    data += static_cast<char>(source);
    numberOfBits -= 8;
    if (numberOfBits <= 8)
    {
        data += static_cast<char>(source >> 8);
        return data;
    }
    data += static_cast<char>(source >> 8);
    numberOfBits -= 8;
    if (numberOfBits <= 8)
    {
        data += static_cast<char>(source >> 16);
        return data;
    }
    data += static_cast<char>(source >> 16);
    data += static_cast<char>(source >> 24);

    return data;
}

uint8_t dataToUint8(const QByteArray& data)
{
    return static_cast<uint8_t>(data[0]);
}

uint16_t dataToUint16(const QByteArray& data)
{
    return static_cast<uint16_t>(data[0])
            | static_cast<uint16_t>(data[1] << 8);
}

uint32_t dataToUint32(const QByteArray& data)
{
    return static_cast<uint32_t>(data[0])
            | (static_cast<uint32_t>(data[1]) << 8)
            | (static_cast<uint32_t>(data[2]) << 16)
            | (static_cast<uint32_t>(data[3]) << 24);
}

uint64_t dataToUint64(const QByteArray& data)
{
    return static_cast<uint64_t>(data[0])
            | (static_cast<uint64_t>(data[1]) << 8)
            | (static_cast<uint64_t>(data[2]) << 16)
            | (static_cast<uint64_t>(data[3]) << 24)
            | (static_cast<uint64_t>(data[4]) << 32)
            | (static_cast<uint64_t>(data[5]) << 40)
            | (static_cast<uint64_t>(data[6]) << 48)
            | (static_cast<uint64_t>(data[7]) << 56);
}

int dataToVInt(const QByteArray& data)
{
    char num3;
    int num = 0;
    int num2 = 0;
    int i=0;
    do
    {
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
    int i=0;
    do
    {
        num3 = data[i++];
        num |= static_cast<size_t>(num3 & 0x7f) << num2;
        num2 += 7;
    } while ((num3 & 0x80) != 0);
    return num;
}

unsigned getVUint32Size(QByteArray data)
{
    unsigned lensize=0;

    char num3;
    do {
        num3 = data[lensize];
        lensize++;
    } while ((num3 & 0x80) != 0);

    return lensize;
}

QByteArray uint8ToData(uint8_t num)
{
    return QByteArray(1, static_cast<char>(num));
}

QByteArray uint16ToData(uint16_t num)
{
    QByteArray data(2,0);
    data[0] = static_cast<char>(num & 0xFF);
    data[1] = static_cast<char>((num>>8) & 0xFF);
    return data;
}

QByteArray uint32ToData(uint32_t num)
{
    QByteArray data(4,0);
    data[0] = static_cast<char>(num & 0xFF);
    data[1] = static_cast<char>((num>>8) & 0xFF);
    data[2] = static_cast<char>((num>>16) & 0xFF);
    data[3] = static_cast<char>((num>>24) & 0xFF);
    return data;
}

QByteArray uint64ToData(uint64_t num)
{
    QByteArray data(8,0);
    data[0] = static_cast<char>(num & 0xFF);
    data[1] = static_cast<char>((num>>8) & 0xFF);
    data[2] = static_cast<char>((num>>16) & 0xFF);
    data[3] = static_cast<char>((num>>24) & 0xFF);
    data[4] = static_cast<char>((num>>32) & 0xFF);
    data[5] = static_cast<char>((num>>40) & 0xFF);
    data[6] = static_cast<char>((num>>48) & 0xFF);
    data[7] = static_cast<char>((num>>56) & 0xFF);
    return data;
}

QByteArray vintToData(int num)
{
    QByteArray data(sizeof(int), 0);
    // Write the size in a Uint of variable lenght (8-32 bits)
    int i=0;
    while (num >= 0x80)
    {
        data[i] = static_cast<char>(num | 0x80);
        i++;
        num = num >> 7;
    }
    data[i] = static_cast<char>(num);
    data.resize(i+1);
    return data;
}

QByteArray vuintToData(size_t num)
{
    QByteArray data(sizeof(size_t), 0);
    // Write the size in a Uint of variable lenght (8-32 bits)
    int i=0;
    while (num >= 0x80)
    {
        data[i] = static_cast<char>(num | 0x80);
        i++;
        num = num >> 7;
    }
    data[i] = static_cast<char>(num);
    data.resize(i+1);
    return data;
}
