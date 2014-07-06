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

#ifndef CAMERA_H
#define CAMERA_H

#include <QCamera>
#include <QVideoFrame>
#include <QAbstractVideoSurface>
#include "vpx/vpx_image.h"

/**
 * This class is a wrapper to share a camera's captured video frames
 * In Qt cameras normally only send their frames to a single output at a time
 * So you can't, for example, send the frames over the network
 * and output them to a widget on the screen at the same time
 *
 * Instead this class allows objects to surscribe and unsuscribe, starting
 * the camera only when needed, and giving access to the last frame
 **/

class Camera : private QAbstractVideoSurface
{
    Q_OBJECT
public:
    Camera();
    void suscribe(); ///< Call this once before trying to get frames
    void unsuscribe(); ///< Call this once when you don't need frames anymore
    QVideoFrame getLastFrame(); ///< Get the last captured frame
    QImage getLastImage(); ///< Convert the last frame to a QImage (can be expensive !)
    vpx_image getLastVPXImage(); ///< Convert the last frame to a vpx_image (can be expensive !)
    bool isFormatSupported(const QVideoSurfaceFormat & format) const;

private slots:
    void onCameraError(QCamera::Error value);

private:
    bool start(const QVideoSurfaceFormat &format);
    bool present(const QVideoFrame &frame);
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;

private:
    int refcount; ///< Number of users suscribed to the camera
    QCamera *camera;
    QVideoFrame lastFrame;
    int frameFormat;
    QList<QVideoFrame::PixelFormat> supportedFormats;
};

#endif // CAMERA_H
