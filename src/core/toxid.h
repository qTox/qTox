/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "toxpk.h"

#include <QByteArray>
#include <QString>
#include <cstdint>

class ToxId
{
public:
    static constexpr int nospamSize = 4;
    static constexpr int nospamNumHexChars = nospamSize*2;
    static constexpr int checksumSize = 2;
    static constexpr int checksumNumHexChars = checksumSize*2;
    static constexpr int size = 38;
    static constexpr int numHexChars = size*2;

    ToxId();
    ToxId(const ToxId& other);
    explicit ToxId(const QString& id);
    explicit ToxId(const QByteArray& rawId);
    explicit ToxId(const uint8_t* rawId, int len);
    ToxId& operator=(const ToxId& other) = default;
    ToxId& operator=(ToxId&& other) = default;

    bool operator==(const ToxId& other) const;
    bool operator!=(const ToxId& other) const;
    QString toString() const;
    void clear();
    bool isValid() const;

    static bool isValidToxId(const QString& id);
    static bool isToxId(const QString& id);
    const uint8_t* getBytes() const;
    QByteArray getToxId() const;
    ToxPk getPublicKey() const;
    QString getNoSpamString() const;

private:
    void constructToxId(const QByteArray& rawId);

public:
    static const QRegularExpression ToxIdRegEx;

private:
    QByteArray toxId;
};
