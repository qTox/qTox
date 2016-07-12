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

/// Describes a video mode supported by a device
struct VideoMode
{
    unsigned short width, height; ///< Displayed video resolution (NOT frame resolution)
    unsigned short x, y; ///< Coordinates of upper-left corner
    float FPS; ///< Max frames per second supported by the device at this resolution
    uint32_t pixel_format;

    VideoMode(int width = 0, int height = 0, int x = 0, int y = 0,
              int FPS = 0, int format = 0) :
        width(width), height(height), x(x), y(y),
        FPS(FPS), pixel_format(format)
    {
    }

    VideoMode(QRect rect) :
        width(rect.width()), height(rect.height()),
        x(rect.x()), y(rect.y()),
        FPS(0), pixel_format(0)
    {
    }

    QRect toRect() const
    {
        return QRect(x, y, width, height);
    }

    /// All zeros means a default/unspecified mode
    operator bool() const
    {
        return width || height || FPS;
    }

    bool operator==(const VideoMode& other) const
    {
        return width == other.width
                && height == other.height
                && x == other.x
                && y == other.y
                && FPS == other.FPS
                && pixel_format == other.pixel_format;
    }

    uint32_t norm(const VideoMode& other) const
    {
        return abs(this->width-other.width) + abs(this->height-other.height);
    }
};

#endif // VIDEOMODE_H

