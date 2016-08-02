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

class CameraSource : public VideoSource
{
    Q_OBJECT

public:
    static CameraSource& getInstance();
    static void destroyInstance();
    void open();
    void open(const QString& deviceName);
    void open(const QString& deviceName, VideoMode mode);
    void close();
    bool isOpen();

    // VideoSource interface
    virtual bool subscribe() override;
    virtual void unsubscribe() override;

signals:
    void deviceOpened();

private:
    CameraSource();
    ~CameraSource();
    void stream();
    bool openDevice();
    void closeDevice();

private:
    QFuture<void> streamFuture;
    QString deviceName;
    CameraDevice* device;
    VideoMode mode;
    AVCodecContext* cctx, *cctxOrig;
    int videoStreamIndex;
    QMutex biglock;
    std::atomic_bool _isOpen;
    std::atomic_bool streamBlocker;
    std::atomic_int subscriptions;

    static CameraSource* instance;
};

#endif // CAMERA_H
