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
#include <QtConcurrent/QtConcurrentRun>
#include <QDebug>
#include <QThread>
#include <QTimer>

Camera* Camera::instance = nullptr;

Camera::Camera()
    : refcount(0)
    , workerThread(nullptr)
    , worker(nullptr)
{
    qDebug() << "New Worker";
    worker = new SelfCamWorker(0);
    workerThread = new QThread();

    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, worker, &SelfCamWorker::onStart);
    connect(workerThread, &QThread::finished, worker, &SelfCamWorker::deleteLater);
    connect(workerThread, &QThread::deleteLater, worker, &SelfCamWorker::deleteLater);
    connect(worker, &SelfCamWorker::newFrameAvailable, this, &Camera::onNewFrameAvailable);
    workerThread->start();
}

Camera::~Camera()
{
    workerThread->exit();
    workerThread->deleteLater();
}

void Camera::subscribe()
{
    if (refcount <= 0)
    {
        refcount = 1;

        //mode.res.setWidth(cam.get(CV_CAP_PROP_FRAME_WIDTH));
        //mode.res.setHeight(cam.get(CV_CAP_PROP_FRAME_HEIGHT));

        worker->resume();
    }
    else
        refcount++;
}

void Camera::unsubscribe()
{
    refcount--;

    if (refcount <= 0)
    {
        worker->suspend();
        refcount = 0;
    }
}

cv::Mat Camera::getLastFrame()
{
    cv::Mat frame;
    cam >> frame;

    return frame;
}

vpx_image Camera::getLastVPXImage()
{
    cv::Mat3b frame = getLastFrame();
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
        mutex.lock();
        cam.set(CV_CAP_PROP_FRAME_WIDTH, res.width());
        cam.set(CV_CAP_PROP_FRAME_HEIGHT, res.height());

        double w = cam.get(CV_CAP_PROP_FRAME_WIDTH);
        double h = cam.get(CV_CAP_PROP_FRAME_HEIGHT);

        qDebug() << "PROBING:" << res << " got " << w << h;

        if (w == res.width() && h == res.height())
        {
            modes.append({res, 60}); // assume 60fps for now
        }
        mutex.unlock();
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
        mutex.lock();
        cam.set(CV_CAP_PROP_FRAME_WIDTH, mode.res.width());
        cam.set(CV_CAP_PROP_FRAME_HEIGHT, mode.res.height());

        mode.res.setWidth(cam.get(CV_CAP_PROP_FRAME_WIDTH));
        mode.res.setHeight(cam.get(CV_CAP_PROP_FRAME_HEIGHT));
        mutex.unlock();
        qDebug() << "VIDEO MODE" << mode.res;
    }
}

Camera::VideoMode Camera::getVideoMode()
{
    return VideoMode{QSize(cam.get(CV_CAP_PROP_FRAME_WIDTH), cam.get(CV_CAP_PROP_FRAME_HEIGHT)), 60};
}

void Camera::setProp(Camera::Prop prop, double val)
{
    switch (prop)
    {
    case BRIGHTNESS:
        cam.set(CV_CAP_PROP_BRIGHTNESS, val);
        break;
    case SATURATION:
        cam.set(CV_CAP_PROP_SATURATION, val);
        break;
    case CONTRAST:
        cam.set(CV_CAP_PROP_CONTRAST, val);
        break;
    case HUE:
        cam.set(CV_CAP_PROP_HUE, val);
        break;
    }
}

double Camera::getProp(Camera::Prop prop)
{
    switch (prop)
    {
    case BRIGHTNESS:
        return cam.get(CV_CAP_PROP_BRIGHTNESS);
    case SATURATION:
        return cam.get(CV_CAP_PROP_SATURATION);
    case CONTRAST:
        return cam.get(CV_CAP_PROP_CONTRAST);
    case HUE:
        return cam.get(CV_CAP_PROP_HUE);
    }

    return 0.0;
}

void Camera::onNewFrameAvailable()
{
    emit frameAvailable();
}

void *Camera::getData()
{
    return currFrame.data;
}

int Camera::getDataSize()
{
    return currFrame.total() * currFrame.channels();
}

void Camera::lock()
{
    mutex.lock();

    mode.res.setWidth(currFrame.cols);
    mode.res.setHeight(currFrame.rows);

    if (worker->hasFrame())
        currFrame = worker->deqeueFrame();
}

void Camera::unlock()
{
    mutex.unlock();
}

QSize Camera::resolution()
{
    return QSize(currFrame.cols, currFrame.rows);
}

Camera* Camera::getInstance()
{
    if (!instance)
        instance = new Camera();

    return instance;
}

// ====================
// WORKER
// ====================

SelfCamWorker::SelfCamWorker(int index)
    : clock(nullptr)
    , camIndex(index)
{
}

void SelfCamWorker::onStart()
{
    clock = new QTimer(this);
    clock->setSingleShot(false);
    clock->setInterval(1);

    connect(clock, &QTimer::timeout, this, &SelfCamWorker::doWork);
    clock->start();

    cam.open(camIndex);
}

void SelfCamWorker::_suspend()
{
    qDebug() << "Suspend";
    if (cam.isOpened())
        cam.release();
}

void SelfCamWorker::_resume()
{
    qDebug() << "Resume";
    if (!cam.isOpened())
        cam.open(camIndex);
}

void SelfCamWorker::doWork()
{
    if (!cam.isOpened())
        return;

    cam >> frame;
//qDebug() << "Decoding frame";
    mutex.lock();
    while (qeue.size() > 3)
        qeue.dequeue();

    qeue.enqueue(frame);
    mutex.unlock();

    emit newFrameAvailable();
}

bool SelfCamWorker::hasFrame()
{
    mutex.lock();
    bool b = !qeue.empty();
    mutex.unlock();

    return b;
}

cv::Mat3b SelfCamWorker::deqeueFrame()
{
    mutex.lock();
    cv::Mat3b f = qeue.dequeue();
    mutex.unlock();

    return f;
}

void SelfCamWorker::suspend()
{
    QMetaObject::invokeMethod(this, "_suspend");
}

void SelfCamWorker::resume()
{
    QMetaObject::invokeMethod(this, "_resume");
}
