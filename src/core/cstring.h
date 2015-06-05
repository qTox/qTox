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

#ifndef CSTRING_H
#define CSTRING_H

#include <cstdint>

class QString;
class QByteArray;

class CString
{
public:
    explicit CString(const QString& string);
    explicit CString(const QByteArray& ba_string);
    explicit CString(const CString& cstr);
    ~CString();

    uint8_t* data();
    uint16_t size() const;

    static QString toString(const uint8_t* cMessage, const uint16_t cMessageSize);
    static uint16_t fromString(const QString& message, uint8_t* cMessage);

private:
    const static int MAX_SIZE_OF_UTF8_ENCODED_CHARACTER = 4;

    uint8_t* cString;
    uint16_t cStringSize;
};
#endif // CSTRING_H
