/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H

#include <QMetaType>
#include <QByteArray>
#include <QSize>

#include "vpx/vpx_image.h"

struct VideoFrame
{
    enum ColorFormat
    {
        NONE,
        BGR,
        YUV,
    };

    QByteArray frameData;
    QSize resolution;
    ColorFormat format;

    VideoFrame() : format(NONE) {}
    VideoFrame(QByteArray d, QSize r, ColorFormat f) : frameData(d), resolution(r), format(f) {}

    void invalidate()
    {
        frameData = QByteArray();
        resolution = QSize(-1,-1);
    }

    bool isValid() const
    {
        return !frameData.isEmpty() && resolution.isValid() && format != NONE;
    }

    vpx_image_t createVpxImage() const;
};

Q_DECLARE_METATYPE(VideoFrame)

#endif // VIDEOFRAME_H
