/*
    Copyright © 2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CAMERA_H
#define CAMERA_H

#include <QHash>
#include <QString>
#include <QFuture>
#include <QVector>
#include <atomic>
#include "src/video/videosource.h"
#include "src/video/videomode.h"

class CameraDevice;
struct AVCodecContext;

/**
 * This class is a wrapper to share a camera's captured video frames
 * It allows objects to suscribe and unsuscribe to the stream, starting
 * the camera and streaming new video frames only when needed.
 * This is a singleton, since we can only capture from one
 * camera at the same time without thread-safety issues.
 * The source is lazy in the sense that it will only keep the video
 * device open as long as there are subscribers, the source can be
 * open but the device closed if there are zero subscribers.
 **/

class CameraSource : public VideoSource
{
    Q_OBJECT

public:
    static CameraSource& getInstance();
    static void destroyInstance();
    /// Opens the source for the camera device in argument, in the settings, or the system default
    /// If a device is already open, the source will seamlessly switch to the new device
    void open();
    void open(const QString deviceName);
    void open(const QString deviceName, VideoMode mode);
    void close(); ///< Equivalent to opening the source with the video device "none". Stops streaming.

    // VideoSource interface
    virtual bool subscribe() override;
    virtual void unsubscribe() override;

signals:
    void deviceOpened();

private:
    CameraSource();
    ~CameraSource();
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
    bool openDevice(); ///< Callers must own the biglock. Actually opens the video device and starts streaming.
    void closeDevice(); ///< Callers must own the biglock. Actually closes the video device and stops streaming.

private:
    QVector<std::weak_ptr<VideoFrame>> freelist; ///< Frames that need freeing before we can safely close the device
    QFuture<void> streamFuture; ///< Future of the streaming thread
    QString deviceName; ///< Short name of the device for CameraDevice's open(QString)
    CameraDevice* device; ///< Non-owning pointer to an open CameraDevice, or nullptr. Not atomic, synced with memfences when becomes null.
    VideoMode mode; ///< What mode we tried to open the device in, all zeros means default mode
    AVCodecContext* cctx, *cctxOrig; ///< Codec context of the camera's selected video stream
    int videoStreamIndex; ///< A camera can have multiple streams, this is the one we're decoding
    std::atomic_bool biglock, freelistLock; ///< True when locked. Faster than mutexes for video decoding.
    std::atomic_bool isOpen;
    std::atomic_int subscriptions; ///< Remember how many times we subscribed for RAII

    static CameraSource* instance;
};

#endif // CAMERA_H
