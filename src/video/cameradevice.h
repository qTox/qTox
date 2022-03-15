/*
    Copyright © 2015-2019 by The qTox Project Contributors

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


#pragma once

#include "videomode.h"
#include <QHash>
#include <QMutex>
#include <QString>
#include <QVector>
#include <atomic>

struct AVFormatContext;
struct AVInputFormat;
struct AVDeviceInfoList;
struct AVDictionary;
class Settings;

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

    static QString getDefaultDeviceName(Settings& settings);

    static bool isScreen(const QString& devName);

private:
    CameraDevice(const QString& devName_, AVFormatContext* context_);
    static CameraDevice* open(QString devName, AVDictionary** options);
    static bool getDefaultInputFormat();
    static QVector<QPair<QString, QString>> getRawDeviceListGeneric();
    static QVector<VideoMode> getScreenModes();

public:
    const QString devName;
    AVFormatContext* context;

private:
    std::atomic_int refcount;
    static QHash<QString, CameraDevice*> openDevices;
    static QMutex openDeviceLock, iformatLock;
};
