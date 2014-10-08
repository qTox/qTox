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
    connect(worker, &SelfCamWorker::started, this, &Camera::onWorkerStarted);
    connect(worker, &SelfCamWorker::newFrameAvailable, this, &Camera::onNewFrameAvailable);
    connect(worker, &SelfCamWorker::resProbingFinished, this, &Camera::onResProbingFinished);
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

cv::Mat Camera::getLastFrame()
{
    return currFrame;
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
    , refCount(0)
{
}

void SelfCamWorker::onStart()
{
    clock = new QTimer(this);
    clock->setSingleShot(false);
    clock->setInterval(5);

    connect(clock, &QTimer::timeout, this, &SelfCamWorker::doWork);

    emit started();
}

void SelfCamWorker::_suspend()
{
    qDebug() << "Suspend";
    clock->stop();
    unsubscribe();
}

void SelfCamWorker::_resume()
{
    qDebug() << "Resume";
    subscribe();
    clock->start();
}

void SelfCamWorker::_setProp(int prop, double val)
{
    props[prop] = val;

    if (cam.isOpened())
        cam.set(prop, val);
}

double SelfCamWorker::_getProp(int prop)
{
    if (!props.contains(prop))
    {
        subscribe();
        props[prop] = cam.get(prop);
        unsubscribe();
        qDebug() << "ASKED " << prop << " VAL " << props[prop];
    }

    return props.value(prop);
}

void SelfCamWorker::probeResolutions()
{
    if (resolutions.isEmpty())
    {
        subscribe();

        // probe resolutions (TODO: add more)
        QList<QSize> propbeRes = {
            QSize( 160, 120), // QQVGA
            QSize( 320, 240), // HVGA
            QSize(1024, 768), // XGA
            QSize( 432, 240), // WQVGA
            QSize( 640, 360), // nHD
        };

        for (QSize res : propbeRes)
        {
            _setProp(CV_CAP_PROP_FRAME_WIDTH, res.width());
            _setProp(CV_CAP_PROP_FRAME_HEIGHT, res.height());

            double w = _getProp(CV_CAP_PROP_FRAME_WIDTH);
            double h = _getProp(CV_CAP_PROP_FRAME_HEIGHT);

            qDebug() << "PROBING:" << res << " got " << w << h;

            resolutions.append(QSize(w,h));
        }

        unsubscribe();
    }

    qDebug() << resolutions;

    emit resProbingFinished(resolutions);
}

void SelfCamWorker::applyProps()
{
    if (!cam.isOpened())
        return;

    for(int prop : props.keys())
        cam.set(prop, props.value(prop));
}

void SelfCamWorker::subscribe()
{
    if (refCount == 0)
    {
        if (!cam.isOpened())
        {
            cam.open(camIndex);
            applyProps(); // restore props
        }
    }

    refCount++;
}

void SelfCamWorker::unsubscribe()
{
    refCount--;

    if(refCount <= 0)
    {
        cam.release();
    }
}

void SelfCamWorker::doWork()
{
    if (!cam.isOpened())
        return;

    if (qeue.size() > 3)
    {
        qeue.dequeue();
        return;
    }

    cam >> frame;
//qDebug() << "Decoding frame";
    mutex.lock();

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

void SelfCamWorker::setProp(int prop, double val)
{
    QMetaObject::invokeMethod(this, "_setProp", Q_ARG(int, prop), Q_ARG(double, val));
}

double SelfCamWorker::getProp(int prop)
{
    double ret = 0.0;
    QMetaObject::invokeMethod(this, "_getProp", Qt::BlockingQueuedConnection, Q_RETURN_ARG(double, ret), Q_ARG(int, prop));

    return ret;
}
