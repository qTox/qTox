/*
    Copyright © 2015 by The qTox Project

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


#include "toxid.h"

#include "core.h"

#include <tox/tox.h>
#include <qregularexpression.h>

#define TOX_ID_PUBLIC_KEY_LENGTH 64
#define TOX_ID_NO_SPAM_LENGTH    8
#define TOX_ID_CHECKSUM_LENGTH   4
#define TOX_HEX_ID_LENGTH 2*TOX_ADDRESS_SIZE

ToxId::ToxId()
: publicKey(), noSpam(), checkSum()
{}

ToxId::ToxId(const ToxId &other)
: publicKey(other.publicKey), noSpam(other.noSpam), checkSum(other.checkSum)
{}

ToxId::ToxId(const QString &id)
{
    if (isToxId(id))
    {
        publicKey = id.left(TOX_ID_PUBLIC_KEY_LENGTH);
        noSpam    = id.mid(TOX_ID_PUBLIC_KEY_LENGTH, TOX_ID_NO_SPAM_LENGTH);
        checkSum  = id.mid(TOX_ID_PUBLIC_KEY_LENGTH + TOX_ID_NO_SPAM_LENGTH, TOX_ID_CHECKSUM_LENGTH);
    }
    else
    {
        publicKey = id;
    }
}

bool ToxId::operator==(const ToxId& other) const
{
    return publicKey == other.publicKey;
}

bool ToxId::operator!=(const ToxId &other) const
{
    return publicKey != other.publicKey;
}

bool ToxId::isActiveProfile() const
{
    return *this == Core::getInstance()->getSelfId();
}

QString ToxId::toString() const
{
    return publicKey + noSpam + checkSum;
}

void ToxId::clear()
{
    publicKey.clear();
    noSpam.clear();
    checkSum.clear();
}

bool ToxId::isToxId(const QString &id)
{
    const QRegularExpression hexRegExp("^[A-Fa-f0-9]+$");
    return id.length() == TOX_HEX_ID_LENGTH && id.contains(hexRegExp);
}
