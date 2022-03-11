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

#include <QColor>
#include <QImage>

class Identicon
{
public:
    Identicon(const QByteArray& data);
    QImage toImage(int scaleFactor = 1);
    static qreal bytesToColor(QByteArray bytes);

public:
    static constexpr int IDENTICON_ROWS = 5;
    static constexpr int IDENTICON_COLOR_BYTES = 6;

private:
    static constexpr int COLORS = 2;
    static constexpr int ACTIVE_COLS = (IDENTICON_ROWS + 1) / 2;
    static constexpr int HASH_MIN_LEN = ACTIVE_COLS * IDENTICON_ROWS
                                      + COLORS * IDENTICON_COLOR_BYTES;

    uint8_t identiconColors[IDENTICON_ROWS][ACTIVE_COLS];
    QColor colors[COLORS];
};
