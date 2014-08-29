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
#include <QVideoSurfaceFormat>
#include <QMessageBox>
#include <QVideoEncoderSettings>
#include <QVideoEncoderSettingsControl>

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
    return frame;
}

QImage Camera::getLastImage()
{
    Mat3b src = getLastFrame();
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
    for (int y = 0; y < src.rows; ++y)
    {
            const cv::Vec3b *srcrow = src[y];
            QRgb *destrow = (QRgb*)dest.scanLine(y);
            for (int x = 0; x < src.cols; ++x)
                    destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
    }
    return dest;
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
