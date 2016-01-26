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

/// Describes a video mode supported by a device
struct VideoMode
{
    unsigned short width, height; ///< Displayed video resolution (NOT frame resolution)
    float FPS; ///< Max frames per second supported by the device at this resolution
    uint32_t pixel_format;

    /// All zeros means a default/unspecified mode
    operator bool() const
    {
        return width || height || FPS;
    }

    bool operator==(const VideoMode& other) const
    {
        return width == other.width
                && height == other.height
                && FPS == other.FPS
                && pixel_format == other.pixel_format;
    }
};

#endif // VIDEOMODE_H

