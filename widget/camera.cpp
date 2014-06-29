#include "camera.h"
#include <QVideoSurfaceFormat>
#include <QMessageBox>

Camera::Camera()
    : refcount{0}, camera{new QCamera}
{
    camera->setCaptureMode(QCamera::CaptureVideo);
    camera->setViewfinder(this);

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
    frameMap.map(QAbstractVideoBuffer::ReadOnly);
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
        QMessageBox::warning(0, "Camera eror",
                QString("Camera format %1 not supported, can't use the camera")
                .arg(format.pixelFormat()));
        return false;
    }
}


QImage Camera::getLastImage()
{
    lastFrame.map(QAbstractVideoBuffer::ReadOnly);
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
                float Y = yData[i*bpl + j];
                float U = uData[i*cxbpl/2 + j/2];
                float V = vData[i*cxbpl/2 + j/2];

                uint8_t R = qMax(qMin((int)(Y + 1.402 * (V - 128)),255),0);
                uint8_t G = qMax(qMin((int)(Y - 0.344 * (U - 128) - 0.714 * (V - 128)),255),0);
                uint8_t B = qMax(qMin((int)(Y + 1.772 * (U - 128)),255),0);

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
                float Y = yData[i*bpl + j];
                float U = uData[i*cxbpl/2 + j/2];
                float V = vData[i*cxbpl/2 + j/2];

                uint8_t R = qMax(qMin((int)(Y + 1.402 * (V - 128)),255),0);
                uint8_t G = qMax(qMin((int)(Y - 0.344 * (U - 128) - 0.714 * (V - 128)),255),0);
                uint8_t B = qMax(qMin((int)(Y + 1.772 * (U - 128)),255),0);

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
