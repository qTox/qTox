/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include "vpx/vpx_image.h"

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    VideoSurface();
    bool start(const QVideoSurfaceFormat &format);
    bool present(const QVideoFrame &frame);
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;

signals:
    // Slots MUST be called with a direct or blocking connection, or img may die before they return !
    void videoFrameReady(vpx_image img);

private:
    QVideoSurfaceFormat mVideoFormat;
    vpx_image_t input;
};

#endif // VIDEOSURFACE_H
