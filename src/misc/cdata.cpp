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

#include "cdata.h"
#include <QString>
#include <tox/tox.h>

// CData

CData::CData(const QString &data, uint16_t byteSize)
{
    cData = new uint8_t[byteSize+1];
    cDataSize = fromString(data, cData);
}

CData::~CData()
{
    delete[] cData;
}

uint8_t* CData::data()
{
    return cData;
}

uint16_t CData::size()
{
    return cDataSize;
}

QString CData::toString(const uint8_t *cData, const uint16_t cDataSize)
{
    return QString(QByteArray(reinterpret_cast<const char*>(cData), cDataSize).toHex()).toUpper();
}

uint16_t CData::fromString(const QString& data, uint8_t* cData)
{
    QByteArray arr = QByteArray::fromHex(data.toLower().toLatin1());
    memcpy(cData, reinterpret_cast<uint8_t*>(arr.data()), arr.size());
    return arr.size();
}


// CUserId

const uint16_t CUserId::SIZE{TOX_CLIENT_ID_SIZE};

CUserId::CUserId(const QString &userId) :
    CData(userId, SIZE)
{
    // intentionally left empty
}

QString CUserId::toString(const uint8_t* cUserId)
{
    return CData::toString(cUserId, SIZE);
}


// CFriendAddress

const uint16_t CFriendAddress::SIZE{TOX_FRIEND_ADDRESS_SIZE};

CFriendAddress::CFriendAddress(const QString &friendAddress) :
    CData(friendAddress, SIZE)
{
    // intentionally left empty
}

QString CFriendAddress::toString(const uint8_t *cFriendAddress)
{
    return CData::toString(cFriendAddress, SIZE);
}
