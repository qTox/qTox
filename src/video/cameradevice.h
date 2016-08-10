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


#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <QHash>
#include <QString>
#include <QMutex>
#include <QVector>
#include <atomic>
#include "videomode.h"

struct AVFormatContext;
struct AVInputFormat;
struct AVDeviceInfoList;
struct AVDictionary;

class CameraDevice
{
public:
    static CameraDevice* open(QString devName, VideoMode mode = VideoMode());
    void open();
    bool close();

    static QVector<QPair<QString, QString>> getDeviceList();

    static QVector<VideoMode> getVideoModes(QString devName);
    static QString getPixelFormatString(uint32_t pixel_format);
    static bool betterPixelFormat(uint32_t a, uint32_t b);

    static QString getDefaultDeviceName();

    static bool isScreen(const QString &devName);

private:
    CameraDevice(const QString &devName, AVFormatContext *context);
    static CameraDevice* open(QString devName, AVDictionary** options);
    static bool getDefaultInputFormat();
    static QVector<QPair<QString, QString> > getRawDeviceListGeneric();
    static QVector<VideoMode> getScreenModes();

public:
    const QString devName;
    AVFormatContext* context;

private:
    std::atomic_int refcount;
    static QHash<QString, CameraDevice*> openDevices;
    static QMutex openDeviceLock, iformatLock;
    static AVInputFormat* iformat, *idesktopFormat;
};

#endif // CAMERADEVICE_H
