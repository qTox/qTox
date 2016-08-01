/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
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

#include "cdata.h"
#include <QString>
#include <tox/tox.h>

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

uint16_t CData::size() const
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

const uint16_t CUserId::SIZE{TOX_PUBLIC_KEY_SIZE};

CUserId::CUserId(const QString &userId) :
    CData(userId, SIZE < userId.size() ? userId.size() : SIZE)
{
    // intentionally left empty
}

QString CUserId::toString(const uint8_t* cUserId)
{
    return CData::toString(cUserId, SIZE);
}


const uint16_t CFriendAddress::SIZE{TOX_ADDRESS_SIZE};

CFriendAddress::CFriendAddress(const QString &friendAddress) :
    CData(friendAddress, SIZE)
{
    // intentionally left empty
}

QString CFriendAddress::toString(const uint8_t *cFriendAddress)
{
    return CData::toString(cFriendAddress, SIZE);
}
