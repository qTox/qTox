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

#include "camera.h"
#include "widget.h"
#include <QDebug>

using namespace cv;

Camera::Camera()
    : refcount{0}
{
}

void Camera::suscribe()
{
    if (refcount <= 0)
    {
        refcount = 1;
        cam.open(0);
    }
    else
        refcount++;
}

void Camera::unsuscribe()
{
    refcount--;

    if (refcount <= 0)
    {
        cam.release();
        refcount = 0;
    }
}

Mat Camera::getLastFrame()
{
    Mat frame;
    cam >> frame;

    Mat out;
    if (!frame.empty())
        cv::cvtColor(frame, out, CV_BGR2RGB);

    return out;
}

vpx_image Camera::getLastVPXImage()
{
    Mat3b frame = getLastFrame();
    vpx_image img;
    int w = frame.size().width, h = frame.size().height;
    vpx_img_alloc(&img, VPX_IMG_FMT_I420, w, h, 1); // I420 == YUV420P, same as YV12 with U and V switched

    size_t i=0, j=0;
    for( int line = 0; line < h; ++line )
    {
        const cv::Vec3b *srcrow = frame[line];
        if( !(line % 2) )
        {
            for( int x = 0; x < w; x += 2 )
            {
                uint8_t r = srcrow[x][2];
                uint8_t g = srcrow[x][1];
                uint8_t b = srcrow[x][0];

                img.planes[VPX_PLANE_Y][i] = ((66*r + 129*g + 25*b) >> 8) + 16;
                img.planes[VPX_PLANE_V][j] = ((-38*r + -74*g + 112*b) >> 8) + 128;
                img.planes[VPX_PLANE_U][j] = ((112*r + -94*g + -18*b) >> 8) + 128;
                i++;
                j++;

                r = srcrow[x+1][2];
                g = srcrow[x+1][1];
                b = srcrow[x+1][0];
                img.planes[VPX_PLANE_Y][i] = ((66*r + 129*g + 25*b) >> 8) + 16;
                i++;
            }
        }
        else
        {
            for( int x = 0; x < w; x += 1 )
            {
                uint8_t r = srcrow[x][2];
                uint8_t g = srcrow[x][1];
                uint8_t b = srcrow[x][0];

                img.planes[VPX_PLANE_Y][i] = ((66*r + 129*g + 25*b) >> 8) + 16;
                i++;
            }
        }
    }
    return img;
}

QList<Camera::VideoMode> Camera::getVideoModes()
{
    // probe resolutions
    QList<QSize> resolutions = {
        QSize( 160, 120), // QQVGA
        QSize( 320, 240), // HVGA
        QSize(1024, 768), // XGA
        QSize( 432, 240), // WQVGA
        QSize( 640, 360), // nHD
    };

    QList<VideoMode> modes;

    for (QSize res : resolutions)
    {
        cam.set(CV_CAP_PROP_FRAME_WIDTH, res.width());
        cam.set(CV_CAP_PROP_FRAME_HEIGHT, res.height());

        double w = cam.get(CV_CAP_PROP_FRAME_WIDTH);
        double h = cam.get(CV_CAP_PROP_FRAME_HEIGHT);

        if (w == res.width() && h == res.height())
        {
            modes.append({res, 60}); // assume 60fps for now
        }
    }

    return modes;
}

Camera::VideoMode Camera::getBestVideoMode()
{
    int bestScore = 0;
    VideoMode bestMode;

    for (VideoMode mode : getVideoModes())
    {
        int score = mode.res.width() * mode.res.height();

        if (score > bestScore)
        {
            bestScore = score;
            bestMode = mode;
        }
    }

    return bestMode;
}

void Camera::setVideoMode(Camera::VideoMode mode)
{
    if (cam.isOpened())
    {
        cam.set(CV_CAP_PROP_FRAME_WIDTH, mode.res.width());
        cam.set(CV_CAP_PROP_FRAME_HEIGHT, mode.res.height());
    }
}

Camera::VideoMode Camera::getVideoMode()
{
    return VideoMode{QSize(cam.get(CV_CAP_PROP_FRAME_WIDTH), cam.get(CV_CAP_PROP_FRAME_HEIGHT)), 60};
}

Camera* Camera::getInstance()
{
    return Widget::getInstance()->getCamera();
}
