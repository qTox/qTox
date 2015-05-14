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

#include <QHash>
#include <QString>
#include <QFuture>
#include <QVector>
#include <atomic>
#include "src/video/videosource.h"

class CameraDevice;
struct AVCodecContext;

/**
 * This class is a wrapper to share a camera's captured video frames
 * It allows objects to suscribe and unsuscribe to the stream, starting
 * the camera and streaming new video frames only when needed.
 **/

class CameraSource : public VideoSource
{
    Q_OBJECT
public:
    CameraSource(); ///< Opens the camera device in the settings, or the system default
    CameraSource(const QString deviceName);
    ~CameraSource();

    // VideoSource interface
    virtual bool subscribe() override;
    virtual void unsubscribe() override;

private:
    /// Blocking. Decodes video stream and emits new frames.
    /// Designed to run in its own thread.
    void stream();
    /// All VideoFrames must be deleted or released before we can close the device
    /// or the device will forcibly free them, and then ~VideoFrame() will double free.
    /// In theory very careful coding from our users could ensure all VideoFrames
    /// die before unsubscribing, even the ones currently in flight in the metatype system.
    /// But that's just asking for trouble and mysterious crashes, so we'll just
    /// maintain a freelist and have all VideoFrames tell us when they die so we can forget them.
    void freelistCallback(int freelistIndex);
    /// Get the index of a free slot in the freelist
    /// Callers must hold the freelistLock
    int getFreelistSlotLockless();

private:
    QVector<std::weak_ptr<VideoFrame>> freelist; ///< Frames that need freeing before we can safely close the device
    QFuture<void> streamFuture; ///< Future of the streaming thread
    const QString deviceName; ///< Short name of the device for CameraDevice's open(QString)
    CameraDevice* device; ///< Non-owning pointer to an open CameraDevice, or nullptr
    AVCodecContext* cctx, *cctxOrig; ///< Codec context of the camera's selected video stream
    int videoStreamIndex; ///< A camera can have multiple streams, this is the one we're decoding
    std::atomic_bool biglock, freelistLock; ///< True when locked. Faster than mutexes for video decoding.
    std::atomic_int subscriptions; ///< Remember how many times we subscribed for RAII
};

#endif // CAMERA_H
