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

#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QQueue>
#include <QSize>

#include "opencv2/opencv.hpp"

class QTimer;

class CameraWorker : public QObject
{
    Q_OBJECT
public:
    CameraWorker(int index);
    void doWork();
    bool hasFrame();
    cv::Mat3b dequeueFrame();

    void suspend();
    void resume();
    void setProp(int prop, double val);
    double getProp(int prop); // blocking call!
    void probeResolutions();

public slots:
    void onStart();

signals:
    void started();
    void newFrameAvailable();
    void resProbingFinished(QList<QSize> res);

private slots:
    void _suspend();
    void _resume();
    void _setProp(int prop, double val);
    double _getProp(int prop);

private:
    void applyProps();
    void subscribe();
    void unsubscribe();

private:
    QMutex mutex;
    QQueue<cv::Mat3b> queue;
    QTimer* clock;
    cv::VideoCapture cam;
    cv::Mat3b frame;
    int camIndex;
    QMap<int, double> props;
    QList<QSize> resolutions;
    int refCount;
};

#endif // CAMERAWORKER_H
