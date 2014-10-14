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

#include "cameraworker.h"

#include <QTimer>
#include <QDebug>

CameraWorker::CameraWorker(int index)
    : clock(nullptr)
    , camIndex(index)
    , refCount(0)
{
    qRegisterMetaType<VideoFrame>();
}

void CameraWorker::onStart()
{
    clock = new QTimer(this);
    clock->setSingleShot(false);
    clock->setInterval(1000/60);

    connect(clock, &QTimer::timeout, this, &CameraWorker::doWork);

    emit started();
}

void CameraWorker::_suspend()
{
    qDebug() << "CameraWorker: Suspend";
    clock->stop();
    unsubscribe();
}

void CameraWorker::_resume()
{
    qDebug() << "CameraWorker: Resume";
    subscribe();
    clock->start();
}

void CameraWorker::_setProp(int prop, double val)
{
    props[prop] = val;

    if (cam.isOpened())
        cam.set(prop, val);
}

double CameraWorker::_getProp(int prop)
{
    if (!props.contains(prop))
    {
        subscribe();
        props[prop] = cam.get(prop);
        unsubscribe();
    }

    return props.value(prop);
}

void CameraWorker::probeResolutions()
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
            cam.set(CV_CAP_PROP_FRAME_WIDTH, res.width());
            cam.set(CV_CAP_PROP_FRAME_HEIGHT, res.height());

            double w = cam.get(CV_CAP_PROP_FRAME_WIDTH);
            double h = cam.get(CV_CAP_PROP_FRAME_HEIGHT);

            //qDebug() << "PROBING:" << res << " got " << w << h;

            if (!resolutions.contains(QSize(w,h)))
                resolutions.append(QSize(w,h));
        }

        unsubscribe();
    }

    qDebug() << resolutions;

    emit resProbingFinished(resolutions);
}

void CameraWorker::applyProps()
{
    if (!cam.isOpened())
        return;

    for(int prop : props.keys())
        cam.set(prop, props.value(prop));
}

void CameraWorker::subscribe()
{
    if (refCount == 0)
    {
        if (!cam.isOpened())
        {
            queue.clear();
            cam.open(camIndex);
            applyProps(); // restore props
        }
    }

    refCount++;
}

void CameraWorker::unsubscribe()
{
    refCount--;

    if(refCount <= 0)
    {
        cam.release();
    }
}

void CameraWorker::doWork()
{
    if (!cam.isOpened())
        return;

    if (!cam.read(frame))
    {
        qDebug() << "CameraWorker: Cannot read frame";
        return;
    }

    QByteArray frameData(reinterpret_cast<char*>(frame.data), frame.total() * frame.channels());

    emit newFrameAvailable(VideoFrame{frameData, QSize(frame.cols, frame.rows), VideoFrame::BGR});
}

void CameraWorker::suspend()
{
    QMetaObject::invokeMethod(this, "_suspend");
}

void CameraWorker::resume()
{
    QMetaObject::invokeMethod(this, "_resume");
}

void CameraWorker::setProp(int prop, double val)
{
    QMetaObject::invokeMethod(this, "_setProp", Q_ARG(int, prop), Q_ARG(double, val));
}

double CameraWorker::getProp(int prop)
{
    double ret = 0.0;
    QMetaObject::invokeMethod(this, "_getProp", Qt::BlockingQueuedConnection, Q_RETURN_ARG(double, ret), Q_ARG(int, prop));

    return ret;
}
