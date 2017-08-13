/*
    Copyright © 2017 by The qTox Project Contributors

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


#include "message.h"

/**
 * @class Message
 * @brief Represents message in chat (including system messages)
 * @var time Date, when message was created. Needed to compare messages
 */

/**
 * @brief Message contains only date, which is needed to sort messages
 */
Message::Message(const QDateTime& time)
    : time{time}
{
}

void Message::setTime(const QDateTime& time)
{
    this->time = time;
}

const QDateTime& Message::getTime() const
{
    return time;
}

bool Message::operator<(const Message& other)
{
    return time < other.time;
}
