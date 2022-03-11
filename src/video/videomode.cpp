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

#include "videomode.h"

/**
 * @struct VideoMode
 * @brief Describes a video mode supported by a device.
 *
 * @var unsigned short VideoMode::width, VideoMode::height
 * @brief Displayed video resolution (NOT frame resolution).
 *
 * @var unsigned short VideoMode::x, VideoMode::y
 * @brief Coordinates of upper-left corner.
 *
 * @var float VideoMode::FPS
 * @brief Frames per second supported by the device at this resolution
 * @note a value < 0 indicates an invalid value
 */

VideoMode::VideoMode(int width_, int height_, int x_, int y_, float FPS_)
    : width(width_)
    , height(height_)
    , x(x_)
    , y(y_)
    , FPS(FPS_)
{
}

VideoMode::VideoMode(QRect rect)
    : width(rect.width())
    , height(rect.height())
    , x(rect.x())
    , y(rect.y())
{
}

QRect VideoMode::toRect() const
{
    return QRect(x, y, width, height);
}

bool VideoMode::operator==(const VideoMode& other) const
{
    return width == other.width && height == other.height && x == other.x && y == other.y
           && qFuzzyCompare(FPS, other.FPS) && pixel_format == other.pixel_format;
}

uint32_t VideoMode::norm(const VideoMode& other) const
{
    return qAbs(width - other.width) + qAbs(height - other.height);
}

uint32_t VideoMode::tolerance() const
{
    constexpr uint32_t minTolerance = 300; // keep wider tolerance for low res cameras
    constexpr uint32_t toleranceFactor = 10; // video mode must be within 10% to be "close enough" to ideal
    return std::max((width + height)/toleranceFactor, minTolerance);
}

/**
 * @brief All zeros means a default/unspecified mode
 */
VideoMode::operator bool() const
{
    return width || height || static_cast<int>(FPS);
}
