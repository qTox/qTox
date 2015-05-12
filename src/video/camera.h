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

#ifndef CAMERA_H
#define CAMERA_H

#include <QImage>
#include <QList>
#include <QMutex>
#include "vpx/vpx_image.h"
#include "opencv2/highgui/highgui.hpp"
#include "src/video/videosource.h"

class CameraWorker;

/**
 * This class is a wrapper to share a camera's captured video frames
 * It allows objects to suscribe and unsuscribe to the stream, starting
 * the camera only when needed, and giving access to the last frames
 **/

class Camera : public VideoSource
{
    Q_OBJECT
public:
    enum Prop : int {
        BRIGHTNESS = CV_CAP_PROP_BRIGHTNESS,
        SATURATION = CV_CAP_PROP_SATURATION,
        CONTRAST = CV_CAP_PROP_CONTRAST,
        HUE = CV_CAP_PROP_HUE,
        WIDTH = CV_CAP_PROP_FRAME_WIDTH,
        HEIGHT = CV_CAP_PROP_FRAME_HEIGHT,
    };

    ~Camera();

    static Camera* getInstance(); ///< Returns the global widget's Camera instance
    VideoFrame getLastFrame();

    void setResolution(QSize res);
    QSize getCurrentResolution();

    void setProp(Prop prop, double val);
    double getProp(Prop prop);

    void probeProp(Prop prop);
    void probeResolutions();

    // VideoSource interface
    virtual void subscribe();
    virtual void unsubscribe();

signals:
    void resolutionProbingFinished(QList<QSize> res);
    void propProbingFinished(Prop prop, double val);

protected:
    Camera();

private:
    int refcount; ///< Number of users suscribed to the camera
    VideoFrame currFrame;
    QMutex mutex;

    QThread* workerThread;
    CameraWorker* worker;

private slots:
    void onNewFrameAvailable(const VideoFrame& frame);

};

#endif // CAMERA_H
