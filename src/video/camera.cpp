/*
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
#include "src/video/cameraworker.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>

Camera* Camera::getInstance()
{
    static Camera instance;

    return &instance;
}

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
    connect(worker, &CameraWorker::newFrameAvailable, this, &Camera::onNewFrameAvailable);

    connect(worker, &CameraWorker::resProbingFinished, this, &Camera::resolutionProbingFinished);
    connect(worker, &CameraWorker::propProbingFinished, this, [=](int prop, double val) { emit propProbingFinished(Prop(prop), val); } );

    workerThread->start();
}

Camera::~Camera()
{
    workerThread->exit();
    workerThread->deleteLater();
}

void Camera::subscribe()
{
    if (refcount++ <= 0)
        worker->resume();
}

void Camera::unsubscribe()
{
    if (--refcount <= 0)
    {
        worker->suspend();
        refcount = 0;
    }
}

void Camera::probeProp(Camera::Prop prop)
{
    worker->probeProp(int(prop));
}

void Camera::probeResolutions()
{
    worker->probeResolutions();
}

void Camera::setResolution(QSize res)
{
    worker->setProp(CV_CAP_PROP_FRAME_WIDTH, res.width());
    worker->setProp(CV_CAP_PROP_FRAME_HEIGHT, res.height());
}

QSize Camera::getCurrentResolution()
{
    return QSize(worker->getProp(CV_CAP_PROP_FRAME_WIDTH), worker->getProp(CV_CAP_PROP_FRAME_HEIGHT));
}

void Camera::setProp(Camera::Prop prop, double val)
{
    worker->setProp(int(prop), val);
}

double Camera::getProp(Camera::Prop prop)
{
    return worker->getProp(int(prop));
}

void Camera::onNewFrameAvailable(const VideoFrame &frame)
{
    emit frameAvailable(frame);

    mutex.lock();
    currFrame = frame;
    mutex.unlock();
}

VideoFrame Camera::getLastFrame()
{
    QMutexLocker lock(&mutex);
    return currFrame;
}
