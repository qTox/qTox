/*
    Copyright © 2020 by The qTox Project Contributors

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

#include <cstdint>
/**
* @struct ToxYUVFrame
* @brief A simple structure to represent a ToxYUV video frame (corresponds to a frame encoded
* under format: AV_PIX_FMT_YUV420P [FFmpeg] or VPX_IMG_FMT_I420 [WebM]).
*
* This structure exists for convenience and code clarity when ferrying YUV420 frames from one
* source to another. The buffers pointed to by the struct should not be owned by the struct nor
* should they be freed from the struct, instead this struct functions only as a simple alias to a
* more complicated frame container.
*/
struct ToxYUVFrame
{
public:
    const uint8_t* y_plane;
    const uint8_t* u_plane;
    const uint8_t* v_plane;

    const uint16_t width;
    const uint16_t height;

    explicit operator bool() const {
            return width > 0 && height > 0;
    };
};

/* This is the type describing frames coming from CoreAV */
struct ToxStridedYUVFrame
{
public:
    const uint8_t* y_plane;
    const uint8_t* u_plane;
    const uint8_t* v_plane;

    const int32_t y_stride;
    const int32_t u_stride;
    const int32_t v_stride;

    const uint16_t width;
    const uint16_t height;

    explicit operator bool() const {
            return width > 0 && height > 0;
    };
};

class ICoreVideo
{
public:
    virtual ~ICoreVideo() = default;
    /**
     * @brief This function will be called with frames from CoreAV
     * @param frame Describes the frame and its data.
     * @note The implementation MUST copy the frame data (y/u/v_plane) of the
     *       ToxStridedYUV frame if it is used.
     */
    virtual void pushFrame(const ToxStridedYUVFrame& frame) = 0;

    /**
     * @brief Tell others when CoreAV stopps sending frames
     * @param state True when stopped, false when restarting
     */
    virtual void setStopped(bool state) = 0;
};
