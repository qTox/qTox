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

#include "cameraworker.h"

#include <QTimer>
#include <QDebug>
#include <QThread>

CameraWorker::CameraWorker(int index)
    : clock(nullptr)
    , camIndex(index)
    , refCount(0)
{
    qRegisterMetaType<VideoFrame>();
    qRegisterMetaType<QList<QSize>>();
}

CameraWorker::~CameraWorker()
{
    if (clock)
        delete clock;
}

void CameraWorker::onStart()
{
    if (!clock)
    {
        clock = new QTimer(this);
        clock->setSingleShot(false);
        clock->setInterval(1000/60);

        connect(clock, &QTimer::timeout, this, &CameraWorker::doWork);
    }
    emit started();
}

void CameraWorker::_suspend()
{
    qDebug() << "Suspend";
    clock->stop();
    unsubscribe();
}

void CameraWorker::_resume()
{
    qDebug() << "Resume";
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
        emit propProbingFinished(prop, props[prop]);
        unsubscribe();
    }

    return props.value(prop);
}

void CameraWorker::_probeResolutions()
{
    if (resolutions.isEmpty())
    {
        subscribe();

        // probe resolutions
        QList<QSize> propbeRes = {
            QSize( 160, 120), // QQVGA
            QSize( 320, 240), // HVGA
            QSize( 432, 240), // WQVGA
            QSize( 640, 360), // nHD
            QSize( 640, 480),
            QSize( 800, 600),
            QSize( 960, 640),
            QSize(1024, 768), // XGA
            QSize(1280, 720),
            QSize(1280, 1024),
            QSize(1360, 768),
            QSize(1366, 768),
            QSize(1400, 1050),
            QSize(1440, 900),
            QSize(1600, 1200),
            QSize(1680, 1050),
            QSize(1920, 1200),
        };

        for (QSize res : propbeRes)
        {
            cam.set(CV_CAP_PROP_FRAME_WIDTH, res.width());
            cam.set(CV_CAP_PROP_FRAME_HEIGHT, res.height());

            double w = cam.get(CV_CAP_PROP_FRAME_WIDTH);
            double h = cam.get(CV_CAP_PROP_FRAME_HEIGHT);

            //qDebug() << "PROBING:" << res << " got " << w << h;

            if (w>0 && h>0 && !resolutions.contains(QSize(w,h)))
                resolutions.append(QSize(w,h));
        }

        unsubscribe();

        qDebug() << "Resolutions" <<resolutions;
    }

    emit resProbingFinished(resolutions);
}

void CameraWorker::applyProps()
{
    if (!cam.isOpened())
        return;

    for (int prop : props.keys())
        cam.set(prop, props.value(prop));
}

void CameraWorker::subscribe()
{
    if (refCount++ == 0)
    {
        if (!cam.isOpened())
        {
            queue.clear();
            bool bSuccess = false;

            try
            {
                bSuccess = cam.open(camIndex);
            }
            catch( cv::Exception& e )
            {
                qDebug() << "OpenCV exception caught: " << e.what();
            }

            if (!bSuccess)
            {
                qDebug() << "Could not open camera";
            }
            applyProps(); // restore props
        }
    }
}

void CameraWorker::unsubscribe()
{
    if (--refCount <= 0)
    {
        cam.release();
        frame = cv::Mat3b();
        queue.clear();
        refCount = 0;
    }
}

void CameraWorker::doWork()
{
    if (!cam.isOpened())
        return;

    bool bSuccess = false;

    try
    {
            bSuccess = cam.read(frame);
    }
    catch( cv::Exception& e )
    {
        qDebug() << "OpenCV exception caught: " << e.what();;
        this->clock->stop(); // prevent log spamming
        qDebug() << "stopped clock";
    }

    if (!bSuccess)
    {
        qDebug() << "Cannot read frame";
        return;
    }

    QByteArray frameData = QByteArray::fromRawData(reinterpret_cast<char*>(frame.data), frame.total() * frame.channels());

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

void CameraWorker::probeProp(int prop)
{
    QMetaObject::invokeMethod(this, "_getProp", Q_ARG(int, prop));
}

void CameraWorker::probeResolutions()
{
    QMetaObject::invokeMethod(this, "_probeResolutions");
}

double CameraWorker::getProp(int prop)
{
    double ret = 0.0;
    QMetaObject::invokeMethod(this, "_getProp", Qt::BlockingQueuedConnection, Q_RETURN_ARG(double, ret), Q_ARG(int, prop));

    return ret;
}
