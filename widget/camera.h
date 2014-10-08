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

#ifndef CAMERA_H
#define CAMERA_H

#include <QImage>
#include <QList>
#include <QQueue>
#include <QMutex>
#include <QMap>
#include "vpx/vpx_image.h"
#include "opencv2/opencv.hpp"
#include "videosource.h"

/**
 * This class is a wrapper to share a camera's captured video frames
 * It allows objects to suscribe and unsuscribe to the stream, starting
 * the camera only when needed, and giving access to the last frames
 **/

class SelfCamWorker : public QObject
{
    Q_OBJECT
public:
    SelfCamWorker(int index);
    void doWork();
    bool hasFrame();
    cv::Mat3b deqeueFrame();

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
    QQueue<cv::Mat3b> qeue;
    QTimer* clock;
    cv::VideoCapture cam;
    cv::Mat3b frame;
    int camIndex;
    QMap<int, double> props;
    QList<QSize> resolutions;
    int refCount;
};

class Camera : public VideoSource
{
    Q_OBJECT
public:
    enum Prop {
        BRIGHTNESS,
        SATURATION,
        CONTRAST,
        HUE,
    };

    ~Camera();

    static Camera* getInstance(); ///< Returns the global widget's Camera instance

    cv::Mat getLastFrame(); ///< Get the last captured frame
    vpx_image getLastVPXImage(); ///< Convert the last frame to a vpx_image (can be expensive !)

    QList<QSize> getSupportedResolutions();
    QSize getBestVideoMode();

    void setResolution(QSize res);
    QSize getResolution();

    void setProp(Prop prop, double val);
    double getProp(Prop prop);

    // VideoSource interface
    virtual void *getData();
    virtual int getDataSize();
    virtual void lock();
    virtual void unlock();
    virtual QSize resolution();
    virtual void subscribe();
    virtual void unsubscribe();

protected:
    Camera();

private:
    int refcount; ///< Number of users suscribed to the camera
    cv::Mat3b currFrame;
    QMutex mutex;

    QThread* workerThread;
    SelfCamWorker* worker;

    QList<QSize> resolutions;

    static Camera* instance;

private slots:
    void onWorkerStarted();
    void onNewFrameAvailable();
    void onResProbingFinished(QList<QSize> res);

};

#endif // CAMERA_H
