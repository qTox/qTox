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
#include "src/cameraworker.h"
#include <QDebug>
#include <QThread>

Camera* Camera::instance = nullptr;

Camera::Camera()
    : refcount(0)
    , workerThread(nullptr)
    , worker(nullptr)
{
    worker = new CameraWorker(0);
    workerThread = new QThread();

    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, worker, &CameraWorker::onStart);
    connect(workerThread, &QThread::finished, worker, &CameraWorker::deleteLater);
    connect(workerThread, &QThread::deleteLater, worker, &CameraWorker::deleteLater);
    connect(worker, &CameraWorker::started, this, &Camera::onWorkerStarted);
    connect(worker, &CameraWorker::newFrameAvailable, this, &Camera::onNewFrameAvailable);
    connect(worker, &CameraWorker::resProbingFinished, this, &Camera::onResProbingFinished);
    workerThread->start();
}

void Camera::onWorkerStarted()
{
    worker->probeResolutions();
}

Camera::~Camera()
{
    workerThread->exit();
    workerThread->deleteLater();
}

void Camera::subscribe()
{
    if (refcount <= 0)
        worker->resume();

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

vpx_image Camera::getLastVPXImage()
{
    lock();
    vpx_image img;
    int w = currFrame.size().width, h = currFrame.size().height;
    vpx_img_alloc(&img, VPX_IMG_FMT_I420, w, h, 1); // I420 == YUV420P, same as YV12 with U and V switched

    size_t i=0, j=0;
    for( int line = 0; line < h; ++line )
    {
        const cv::Vec3b *srcrow = currFrame[line];
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
    unlock();
    return img;
}

QList<QSize> Camera::getSupportedResolutions()
{
    return resolutions;
}

QSize Camera::getBestVideoMode()
{
    int bestScore = 0;
    QSize bestRes;

    for (QSize res : getSupportedResolutions())
    {
        int score = res.width() * res.height();

        if (score > bestScore)
        {
            bestScore = score;
            bestRes = res;
        }
    }

    return bestRes;
}

void Camera::setResolution(QSize res)
{
    worker->setProp(CV_CAP_PROP_FRAME_WIDTH, res.width());
    worker->setProp(CV_CAP_PROP_FRAME_HEIGHT, res.height());
}

QSize Camera::getResolution()
{
    return QSize(worker->getProp(CV_CAP_PROP_FRAME_WIDTH), worker->getProp(CV_CAP_PROP_FRAME_HEIGHT));
}

void Camera::setProp(Camera::Prop prop, double val)
{
    switch (prop)
    {
    case BRIGHTNESS:
        worker->setProp(CV_CAP_PROP_BRIGHTNESS, val);
        break;
    case SATURATION:
        worker->setProp(CV_CAP_PROP_SATURATION, val);
        break;
    case CONTRAST:
        worker->setProp(CV_CAP_PROP_CONTRAST, val);
        break;
    case HUE:
        worker->setProp(CV_CAP_PROP_HUE, val);
        break;
    }
}

double Camera::getProp(Camera::Prop prop)
{
    switch (prop)
    {
    case BRIGHTNESS:
        return worker->getProp(CV_CAP_PROP_BRIGHTNESS);
    case SATURATION:
        return worker->getProp(CV_CAP_PROP_SATURATION);
    case CONTRAST:
        return worker->getProp(CV_CAP_PROP_CONTRAST);
    case HUE:
        return worker->getProp(CV_CAP_PROP_HUE);
    }

    return 0.0;
}

void Camera::onNewFrameAvailable()
{
    emit frameAvailable();
}

void Camera::onResProbingFinished(QList<QSize> res)
{
    resolutions = res;
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

    if (worker->hasFrame())
        currFrame = worker->dequeueFrame();
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
