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
#include <QMutex>
#include "vpx/vpx_image.h"
#include "opencv2/opencv.hpp"
#include "videosource.h"

/**
 * This class is a wrapper to share a camera's captured video frames
 * It allows objects to suscribe and unsuscribe to the stream, starting
 * the camera only when needed, and giving access to the last frames
 **/

class Camera : public VideoSource
{
public:
    struct VideoMode {
        QSize res;
        double fps;
    };

    enum Prop {
        BRIGHTNESS,
        SATURATION,
        CONTRAST,
        HUE,
    };

    Camera();
    static Camera* getInstance(); ///< Returns the global widget's Camera instance
    virtual void subscribe(); ///< Call this once before trying to get frames
    virtual void unsubscribe(); ///< Call this once when you don't need frames anymore
    cv::Mat getLastFrame(); ///< Get the last captured frame
    vpx_image getLastVPXImage(); ///< Convert the last frame to a vpx_image (can be expensive !)

    QList<VideoMode> getVideoModes();
    VideoMode getBestVideoMode();

    void setVideoMode(VideoMode mode);
    VideoMode getVideoMode();

    void setProp(Prop prop, double val);
    double getProp(Prop prop);

private:
    int refcount; ///< Number of users suscribed to the camera
    cv::VideoCapture cam; ///< OpenCV camera capture opbject
    cv::Mat3b currFrame;
    QMutex mutex;

    // VideoSource interface
public:
    virtual void *getData();
    virtual int getDataSize();
    virtual void lock();
    virtual void unlock();
    virtual QSize resolution();
    virtual double fps();
};

#endif // CAMERA_H
