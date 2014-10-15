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
#include <QMutexLocker>

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
    QMutexLocker lock(&mutex);

    vpx_image img;

    if (currFrame.isNull())
    {
        img.w = 0;
        img.h = 0;
        return img;
    }

    int w = currFrame.resolution.width();
    int h = currFrame.resolution.height();

    // I420 "It comprises an NxM Y plane followed by (N/2)x(M/2) V and U planes."
    // http://fourcc.org/yuv.php#IYUV
    vpx_img_alloc(&img, VPX_IMG_FMT_VPXI420, w, h, 1);

    for (int x = 0; x < w; x += 2)
    {
        for (int y = 0; y < h; y += 2)
        {
            QRgb p1 = currFrame.getPixel(x, y);
            QRgb p2 = currFrame.getPixel(x + 1, y);
            QRgb p3 = currFrame.getPixel(x, y + 1);
            QRgb p4 = currFrame.getPixel(x + 1, y + 1);

            img.planes[VPX_PLANE_Y][x + y * w] = ((66 * qRed(p1) + 129 * qGreen(p1) + 25 * qBlue(p1)) >> 8) + 16;
            img.planes[VPX_PLANE_Y][x + 1 + y * w] = ((66 * qRed(p2) + 129 * qGreen(p2) + 25 * qBlue(p2)) >> 8) + 16;
            img.planes[VPX_PLANE_Y][x + (y + 1) * w] = ((66 * qRed(p3) + 129 * qGreen(p3) + 25 * qBlue(p3)) >> 8) + 16;
            img.planes[VPX_PLANE_Y][x + 1 + (y + 1) * w] = ((66 * qRed(p4) + 129 * qGreen(p4) + 25 * qBlue(p4)) >> 8) + 16;

            if (!(x % 2) && !(y % 2))
            {
                // TODO: consider p1 to p4?

                int i = x / 2;
                int j = y / 2;

                img.planes[VPX_PLANE_U][i + j * w / 2] = ((112 * qRed(p1) + -94 * qGreen(p1) + -18 * qBlue(p1)) >> 8) + 128;
                img.planes[VPX_PLANE_V][i + j * w / 2] = ((-38 * qRed(p1) + -74 * qGreen(p1) + 112 * qBlue(p1)) >> 8) + 128;
            }
        }
    }

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

void Camera::onNewFrameAvailable(const VideoFrame frame)
{
    emit frameAvailable(frame);

    mutex.lock();
    currFrame = frame;
    mutex.unlock();
}

void Camera::onResProbingFinished(QList<QSize> res)
{
    resolutions = res;
}

Camera* Camera::getInstance()
{
    if (!instance)
        instance = new Camera();

    return instance;
}
