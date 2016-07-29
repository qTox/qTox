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

#ifndef VIDEOMODE_H
#define VIDEOMODE_H

#include <QRect>
#include <cstdint>

struct VideoMode
{
    int width, height;
    int x, y;
    float FPS;
    uint32_t pixel_format;

    VideoMode(int width = 0, int height = 0, int x = 0, int y = 0,
              int FPS = 0, int format = 0);

    VideoMode(QRect rect);

    QRect toRect() const;

    operator bool() const;
    bool operator==(const VideoMode& other) const;
    uint32_t norm(const VideoMode& other) const;
};

#endif // VIDEOMODE_H

