/*
    Copyright © 2019 by The qTox Project Contributors

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

#pragma once

#include <QByteArray>
#include <QString>
#include <cstdint>
#include <QHash>
#include <memory>

class ChatId
{
public:
    virtual ~ChatId();
    ChatId(const ChatId&) = default;
    ChatId& operator=(const ChatId&) = default;
    ChatId(ChatId&&) = default;
    ChatId& operator=(ChatId&&) = default;
    bool operator==(const ChatId& other) const;
    bool operator!=(const ChatId& other) const;
    bool operator<(const ChatId& other) const;
    QString toString() const;
    QByteArray getByteArray() const;
    const uint8_t* getData() const;
    bool isEmpty() const;
    virtual int getSize() const = 0;
    virtual std::unique_ptr<ChatId> clone() const = 0;

protected:
    ChatId();
    explicit ChatId(const QByteArray& rawId);
    QByteArray id;
};

inline uint qHash(const ChatId& id)
{
    return qHash(id.getByteArray());
}

using ChatIdPtr = std::shared_ptr<const ChatId>;
