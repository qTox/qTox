/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>

    This file is part of Tox Qt GUI.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "cstring.h"
#include <QString>

CString::CString(const QString& string) :
    CString(string.toUtf8())
{
}

CString::CString(const QByteArray& ba_string)
{
    cString = new uint8_t[ba_string.size()]();
    cStringSize = ba_string.size();
    memcpy(cString, reinterpret_cast<const uint8_t*>(ba_string.data()), cStringSize);
}

CString::CString(const CString &cstr)
{
    cStringSize = cstr.cStringSize;
    cString = new uint8_t[cStringSize]();
    memcpy(cString, cstr.cString, cStringSize);
}

CString::~CString()
{
    delete[] cString;
}

uint8_t* CString::data()
{
    return cString;
}

uint16_t CString::size() const
{
    return cStringSize;
}

QString CString::toString(const uint8_t* cString, uint16_t cStringSize)
{
    return QString::fromUtf8(reinterpret_cast<const char*>(cString), cStringSize);
}

uint16_t CString::fromString(const QString& string, uint8_t* cString)
{
    QByteArray byteArray = QByteArray(string.toUtf8());
    memcpy(cString, reinterpret_cast<uint8_t*>(byteArray.data()), byteArray.size());
    return byteArray.size();
}
