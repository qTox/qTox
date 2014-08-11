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
        camera->start();
    }
    else
        refcount++;
}

void Camera::unsuscribe()
{
    refcount--;

    if (refcount <= 0)
    {
        camera->stop();
        refcount = 0;
    }
}

QVideoFrame Camera::getLastFrame()
{
    return lastFrame;
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
    if (!lastFrame.map(QAbstractVideoBuffer::ReadOnly))
    {
        qWarning() << "Camera::getLastImage: Error maping last frame";
        return QImage();
    }
    int w = lastFrame.width(), h = lastFrame.height();
    int bpl = lastFrame.bytesPerLine(), cxbpl = bpl/2;
    QImage img(w, h, QImage::Format_RGB32);

    if (frameFormat == QVideoFrame::Format_YUV420P)
    {
        uint8_t* yData = lastFrame.bits();
        uint8_t* uData = yData + (bpl * h);
        uint8_t* vData = uData + (bpl * h / 4);
        for (int i = 0; i< h; i++)
        {
            uint32_t* scanline = (uint32_t*)img.scanLine(i);
            for (int j=0; j < bpl; j++)
            {
                uint8_t Y = yData[i*bpl + j];
                uint8_t U = uData[i/2*cxbpl + j/2];
                uint8_t V = vData[i/2*cxbpl + j/2];

                uint8_t R, G, B;
                fromYCbCrToRGB(Y, U, V, R, G, B);

                scanline[j] = (0xFF<<24) + (R<<16) + (G<<8) + B;
            }
        }
    }
    else if (frameFormat == QVideoFrame::Format_YV12)
    {
        uint8_t* yData = lastFrame.bits();
        uint8_t* vData = yData + (bpl * h);
        uint8_t* uData = vData + (bpl * h / 4);
        for (int i = 0; i< h; i++)
        {
            uint32_t* scanline = (uint32_t*)img.scanLine(i);
            for (int j=0; j < bpl; j++)
            {
                uint8_t Y = yData[i*bpl + j];
                uint8_t U = uData[i/2*cxbpl + j/2];
                uint8_t V = vData[i/2*cxbpl + j/2];

                uint8_t R, G, B;
                fromYCbCrToRGB(Y, U, V, R, G, B);

                scanline[j] = (0xFF<<24) + (R<<16) + (G<<8) + B;
            }
        }
    }
    else if (frameFormat == QVideoFrame::Format_RGB32)
    {
        memcpy(img.bits(), lastFrame.bits(), bpl*h);
    }

    lastFrame.unmap();
    return img;
}

vpx_image Camera::getLastVPXImage()
{
    vpx_image img;
    img.w = img.h = 0;
    if (!lastFrame.isValid())
        return img;
    if (!lastFrame.map(QAbstractVideoBuffer::ReadOnly))
    {
        qWarning() << "Camera::getLastVPXImage: Error maping last frame";
        return img;
    }
    int w = lastFrame.width(), h = lastFrame.height();
    int bpl = lastFrame.bytesPerLine();
    vpx_img_alloc(&img, VPX_IMG_FMT_I420, w, h, 1); // I420 == YUV420P, same as YV12 with U and V switched

    if (frameFormat == QVideoFrame::Format_YUV420P)
    {
        uint8_t* yData = lastFrame.bits();
        uint8_t* uData = yData + (bpl * h);
        uint8_t* vData = uData + (bpl * h / 4);
        img.planes[VPX_PLANE_Y] = yData;
        img.planes[VPX_PLANE_U] = uData;
        img.planes[VPX_PLANE_V] = vData;
    }
    else if (frameFormat == QVideoFrame::Format_YV12)
    {
        uint8_t* yData = lastFrame.bits();
        uint8_t* uData = yData + (bpl * h);
        uint8_t* vData = uData + (bpl * h / 4);
        img.planes[VPX_PLANE_Y] = yData;
        img.planes[VPX_PLANE_U] = vData;
        img.planes[VPX_PLANE_V] = uData;
    }
    else if (frameFormat == QVideoFrame::Format_RGB32 || frameFormat == QVideoFrame::Format_ARGB32)
    {
        qWarning() << "Camera::getLastVPXImage: Using experimental RGB32 conversion code";
        uint8_t* rgb = lastFrame.bits();
        size_t i=0, j=0;

        for( int line = 0; line < h; ++line )
        {
            if( !(line % 2) )
            {
                for( int x = 0; x < w; x += 2 )
                {
                    uint8_t r = rgb[4 * i + 1];
                    uint8_t g = rgb[4 * i + 2];
                    uint8_t b = rgb[4 * i + 3];

                    img.planes[VPX_PLANE_Y][i] = ((66*r + 129*g + 25*b) >> 8) + 16;
                    img.planes[VPX_PLANE_U][j] = ((-38*r + -74*g + 112*b) >> 8) + 128;
                    img.planes[VPX_PLANE_V][j] = ((112*r + -94*g + -18*b) >> 8) + 128;
                    i++;
                    j++;

                    r = rgb[4 * i + 1];
                    g = rgb[4 * i + 2];
                    b = rgb[4 * i + 3];

                    img.planes[VPX_PLANE_Y][i] = ((66*r + 129*g + 25*b) >> 8) + 16;
                    i++;
                }
            }
            else
            {
                for( int x = 0; x < w; x += 1 )
                {
                    uint8_t r = rgb[4 * i + 1];
                    uint8_t g = rgb[4 * i + 2];
                    uint8_t b = rgb[4 * i + 3];

                    img.planes[VPX_PLANE_Y][i] = ((66*r + 129*g + 25*b) >> 8) + 16;
                    i++;
                }
            }
        }
    }

    lastFrame.unmap();
    return img;

}
