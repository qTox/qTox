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

#include "timestamp.h"

Timestamp::Timestamp(const QDateTime &time, const QString &format, const QFont &font)
    : Text(time.toString(format), font, false, time.toString(format))
{
    this->time = time;
}

QDateTime Timestamp::getTime()
{
    return time;
}
