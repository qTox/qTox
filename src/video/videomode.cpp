/*
    Copyright © 2015-2016 by The qTox Project

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

#include "videomode.h"

/**
@struct VideoMode
@brief Describes a video mode supported by a device.

@var unsigned short VideoMode::width, VideoMode::height
@brief Displayed video resolution (NOT frame resolution).

@var unsigned short VideoMode::x, VideoMode::y
@brief Coordinates of upper-left corner.

@var float VideoMode::FPS
@brief Frames per second supported by the device at this resolution
*/

VideoMode::VideoMode(int width, int height, int x, int y, int FPS, int format) :
    width(width), height(height), x(x), y(y),
    FPS(FPS), pixel_format(format)
{
}

VideoMode::VideoMode(QRect rect) :
    width(rect.width()), height(rect.height()),
    x(rect.x()), y(rect.y()),
    FPS(0), pixel_format(0)
{
}

QRect VideoMode::toRect() const
{
    return QRect(x, y, width, height);
}

bool VideoMode::operator==(const VideoMode &other) const
{
    return width == other.width
            && height == other.height
            && x == other.x
            && y == other.y
            && FPS == other.FPS
            && pixel_format == other.pixel_format;
}

uint32_t VideoMode::norm(const VideoMode &other) const
{
    return abs(this->width-other.width) + abs(this->height-other.height);
}

/**
@brief All zeros means a default/unspecified mode
*/
VideoMode::operator bool() const
{
    return width || height || FPS;
}
