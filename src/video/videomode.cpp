/*
    Copyright © 2015-2016 by The qTox Project Contributors

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

#include <QDebug>

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
 * @var float VideoMode::defaultFPS
 * @brief The default FPS value for that video mode or <0 if invalid.
 *
 * @var int VideoMode::selectedFPSIdx
 * @brief The selected FPS value for that video mode or <0 if invalid.
 *
 * @var QVector<float> VideoMode::availableFPS
 * @brief Contains all available FPS values for this resolution.
 *
 * @var float VideoMode::FPS
 * @brief Frames per second supported by the device at this resolution
 * @note a value < 0 indicates an invalid value
 */

/**
 * @brief VideoMode::VideoMode
 * @param width Displayed frame width
 * @param height Displayed frame height
 * @param x Coordinate of the upper left corner
 * @param y Coordinate of the upper left corner
 * @param additionalFPS Add a custom FPS value, usefulf for desktop sharing.
 */
VideoMode::VideoMode(int width, int height, int x, int y, float additionalFPS)
    : width(width)
    , height(height)
    , x(x)
    , y(y)
{
    if(additionalFPS < 0) {
        return;
    }

    availableFPS.push_front(additionalFPS);
    selectedFPSIdx = 0;
}

/**
 * @brief VideoMode::VideoMode overloaded constructor
 */
VideoMode::VideoMode(QRect rect, float additionalFPS)
    : VideoMode(rect.width(), rect.height(), rect.x(), rect.y(), additionalFPS)
{
}

QRect VideoMode::toRect() const
{
    return QRect(x, y, width, height);
}

bool VideoMode::operator==(const VideoMode& other) const
{
    // don't compare available FPS vector, so one can easily add FPS values
    // during initialization
    return width == other.width && height == other.height && x == other.x && y == other.y
           && selectedFPSIdx == other.selectedFPSIdx && pixel_format == other.pixel_format;
}

uint32_t VideoMode::norm(const VideoMode& other) const
{
    return qAbs(this->width - other.width) + qAbs(this->height - other.height);
}

/**
 * @brief All zeros means a default/unspecified mode
 */
VideoMode::operator bool() const
{
    return width && height && selectedFPSIdx >= 0;
}
