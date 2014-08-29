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

static inline void fromYCbCrToRGB(
        uint8_t Y, uint8_t Cb, uint8_t Cr,
        uint8_t& R, uint8_t& G, uint8_t& B)
{
    int r = Y + ((1436 * (Cr - 128)) >> 10),
        g = Y - ((354 * (Cb - 128) + 732 * (Cr - 128)) >> 10),
        b = Y + ((1814 * (Cb - 128)) >> 10);

    if(r < 0) {
        r = 0;
    } else if(r > 255) {
        r = 255;
    }

    if(g < 0) {
        g = 0;
    } else if(g > 255) {
        g = 255;
    }

    if(b < 0) {
        b = 0;
    } else if(b > 255) {
        b = 255;
    }

    R = static_cast<uint8_t>(r);
    G = static_cast<uint8_t>(g);
    B = static_cast<uint8_t>(b);
}

Camera::Camera()
    : refcount{0}, camera{new QCamera}
{
    camera->setCaptureMode(QCamera::CaptureVideo);
    camera->setViewfinder(this);

#if 0 // Crashes on Windows
    QMediaService *m = camera->service();
    QVideoEncoderSettingsControl *enc = m->requestControl<QVideoEncoderSettingsControl*>();
    QVideoEncoderSettings sets = enc->videoSettings();
    sets.setResolution(640, 480);
    enc->setVideoSettings(sets);
#endif

    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(onCameraError(QCamera::Error)));

    supportedFormats << QVideoFrame::Format_YUV420P << QVideoFrame::Format_YV12 << QVideoFrame::Format_RGB32;
}

void Camera::suscribe()
{
    if (refcount <= 0)
    {
        refcount = 1;
        cap.open(0);
    }
    else
        refcount++;
}

void Camera::unsuscribe()
{
    refcount--;

    if (refcount <= 0)
    {
        cap.release();
        refcount = 0;
    }
}

Mat Camera::getLastFrame()
{
    Mat frame;
    cap >> frame;
    return frame;
}

bool Camera::start(const QVideoSurfaceFormat &format)
{
    if(supportedFormats.contains(format.pixelFormat()))
    {
        frameFormat = format.pixelFormat();
        QAbstractVideoSurface::start(format);
        return true;
    }
    else
    {
        QMessageBox::warning(0, "Camera error", "The camera only supports rare video formats, can't use it");
        return false;
    }
}

bool Camera::present(const QVideoFrame &frame)
{
    QVideoFrame frameMap(frame); // Basically a const_cast because shallow copies
    if (!frameMap.map(QAbstractVideoBuffer::ReadOnly))
    {
        qWarning() << "Camera::present: Unable to map frame";
        return false;
    }
    int w = frameMap.width(), h = frameMap.height();
    int bpl = frameMap.bytesPerLine(), size = frameMap.mappedBytes();
    QVideoFrame frameCopy(size, QSize(w, h), bpl, frameMap.pixelFormat());
    frameCopy.map(QAbstractVideoBuffer::WriteOnly);
    memcpy(frameCopy.bits(), frameMap.bits(), size);
    frameCopy.unmap();
    lastFrame = frameCopy;
    frameMap.unmap();
    return true;
}

QList<QVideoFrame::PixelFormat> Camera::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle)
        return supportedFormats;
    else
        return QList<QVideoFrame::PixelFormat>();
}

void Camera::onCameraError(QCamera::Error value)
{
    QMessageBox::warning(0,"Camera error",QString("Error %1 : %2")
                         .arg(value).arg(camera->errorString()));
}

bool Camera::isFormatSupported(const QVideoSurfaceFormat& format) const
{
    if (format.pixelFormat() == 0)
    {
        //QMessageBox::warning(0, "Camera eror","The camera's video format is not supported !");
        return QAbstractVideoSurface::isFormatSupported(format);
    }
    else if(supportedFormats.contains(format.pixelFormat()))
    {
        return true;
    }
    else
    {
        QMessageBox::warning(0, tr("Camera eror"),
                tr("Camera format %1 not supported, can't use the camera")
                .arg(format.pixelFormat()));
        return false;
    }
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

    //qWarning() << "Camera::getLastVPXImage: Using experimental RGB32 conversion code" << w << ","<<h;
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
