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

#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
}
#include "cameradevice.h"
#include "src/persistence/settings.h"

#ifdef Q_OS_WIN
#include "src/platform/camera/directshow.h"
#endif
#ifdef Q_OS_LINUX
#include "src/platform/camera/v4l2.h"
#endif
#ifdef Q_OS_OSX
#include "src/platform/camera/avfoundation.h"
#endif

QHash<QString, CameraDevice*> CameraDevice::openDevices;
QMutex CameraDevice::openDeviceLock, CameraDevice::iformatLock;
AVInputFormat* CameraDevice::iformat{nullptr};
AVInputFormat* CameraDevice::idesktopFormat{nullptr};

CameraDevice::CameraDevice(const QString &devName, AVFormatContext *context)
    : devName{devName}, context{context}, refcount{1}
{
}

CameraDevice* CameraDevice::open(QString devName, AVDictionary** options)
{
    openDeviceLock.lock();
    AVFormatContext* fctx = nullptr;
    CameraDevice* dev = openDevices.value(devName);
    int aduration;
    if (dev)
        goto out;

    AVInputFormat* format;
    if (devName.startsWith("x11grab#"))
    {
        devName = devName.mid(8);
        format = idesktopFormat;
    }
    else if (devName.startsWith("gdigrab#"))
    {
        devName = devName.mid(8);
        format = idesktopFormat;
    }
    else
    {
        format = iformat;
    }

    if (avformat_open_input(&fctx, devName.toStdString().c_str(), format, options)<0)
        goto out;

    // Fix avformat_find_stream_info hanging on garbage input
#if FF_API_PROBESIZE_32
    aduration = fctx->max_analyze_duration2 = 0;
#else
    aduration = fctx->max_analyze_duration = 0;
#endif

    if (avformat_find_stream_info(fctx, NULL) < 0)
    {
        avformat_close_input(&fctx);
        goto out;
    }

#if FF_API_PROBESIZE_32
    fctx->max_analyze_duration2 = aduration;
#else
    fctx->max_analyze_duration = aduration;
#endif

    dev = new CameraDevice{devName, fctx};
    openDevices[devName] = dev;

out:
    openDeviceLock.unlock();
    return dev;
}

CameraDevice* CameraDevice::open(QString devName)
{
    VideoMode mode{0,0,0,0};
    return open(devName, mode);
}

CameraDevice* CameraDevice::open(QString devName, VideoMode mode)
{
    if (!getDefaultInputFormat())
        return nullptr;

    if (devName == "none")
    {
        qDebug() << "Tried to open the null device";
        return nullptr;
    }

    AVDictionary* options = nullptr;
    if (!iformat);
#ifdef Q_OS_LINUX
    else if (devName.startsWith("x11grab#"))
    {
        QSize screen;
        if (mode.width && mode.height)
        {
            screen.setWidth(mode.width);
            screen.setHeight(mode.height);
        }
        else
        {
            screen = QApplication::desktop()->screenGeometry().size();
            // Workaround https://trac.ffmpeg.org/ticket/4574 by choping 1 px bottom and right
            // Actually, let's chop two pixels, toxav hates odd resolutions (off by one stride)
            screen.setWidth(screen.width()-2);
            screen.setHeight(screen.height()-2);
        }
        av_dict_set(&options, "video_size", QString("%1x%2").arg(screen.width()).arg(screen.height()).toStdString().c_str(), 0);
        if (mode.FPS)
            av_dict_set(&options, "framerate", QString().setNum(mode.FPS).toStdString().c_str(), 0);
        else
            av_dict_set(&options, "framerate", QString().setNum(5).toStdString().c_str(), 0);
    }
#endif
#ifdef Q_OS_WIN
    else if (devName.startsWith("gdigrab#"))
    {
        av_dict_set(&options, "framerate", QString().setNum(5).toStdString().c_str(), 0);
    }
#endif
#ifdef Q_OS_WIN
    else if (iformat->name == QString("dshow") && mode)
    {
        av_dict_set(&options, "video_size", QString("%1x%2").arg(mode.width).arg(mode.height).toStdString().c_str(), 0);
        av_dict_set(&options, "framerate", QString().setNum(mode.FPS).toStdString().c_str(), 0);
    }
#endif
#ifdef Q_OS_LINUX
    else if (iformat->name == QString("video4linux2,v4l2") && mode)
    {
        av_dict_set(&options, "video_size", QString("%1x%2").arg(mode.width).arg(mode.height).toStdString().c_str(), 0);
        av_dict_set(&options, "framerate", QString().setNum(mode.FPS).toStdString().c_str(), 0);
        const char *pixel_format = v4l2::getPixelFormatString(mode.pixel_format).toStdString().c_str();
        if (strncmp(pixel_format, "unknown", 7) != 0)
        {
            av_dict_set(&options, "pixel_format", pixel_format, 0);
        }
    }
#endif
#ifdef Q_OS_OSX
    else if (iformat->name == QString("avfoundation"))
    {
        if (mode)
        {
            av_dict_set(&options, "video_size", QString("%1x%2").arg(mode.width).arg(mode.height).toStdString().c_str(), 0);
            av_dict_set(&options, "framerate", QString().setNum(mode.FPS).toStdString().c_str(), 0);
        }
        else if (devName.startsWith(avfoundation::CAPTURE_SCREEN))
        {
            av_dict_set(&options, "framerate", QString().setNum(5).toStdString().c_str(), 0);
            av_dict_set_int(&options, "capture_cursor", 1, 0);
            av_dict_set_int(&options, "capture_mouse_clicks", 1, 0);
        }
    }
#endif
    else if (mode)
    {
        qWarning() << "Video mode-setting not implemented for input "<<iformat->name;
        (void)mode;
    }

    CameraDevice* dev = open(devName, &options);
    if (options)
        av_dict_free(&options);
    return dev;
}

void CameraDevice::open()
{
    ++refcount;
}

bool CameraDevice::close()
{
    if (--refcount <= 0)
    {
        openDeviceLock.lock();
        openDevices.remove(devName);
        openDeviceLock.unlock();
        avformat_close_input(&context);
        delete this;
        return true;
    }
    else
    {
        return false;
    }
}

QVector<QPair<QString, QString>> CameraDevice::getRawDeviceListGeneric()
{
    QVector<QPair<QString, QString>> devices;

    if (!getDefaultInputFormat())
        return devices;

    // Alloc an input device context
    AVFormatContext *s;
    if (!(s = avformat_alloc_context()))
        return devices;
    if (!iformat->priv_class || !AV_IS_INPUT_DEVICE(iformat->priv_class->category))
    {
        avformat_free_context(s);
        return devices;
    }
    s->iformat = iformat;
    if (s->iformat->priv_data_size > 0)
    {
        s->priv_data = av_mallocz(s->iformat->priv_data_size);
        if (!s->priv_data)
        {
            avformat_free_context(s);
            return devices;
        }
        if (s->iformat->priv_class)
        {
            *(const AVClass**)s->priv_data= s->iformat->priv_class;
            av_opt_set_defaults(s->priv_data);
        }
    }
    else
    {
        s->priv_data = NULL;
    }

    // List the devices for this context
    AVDeviceInfoList* devlist = nullptr;
    AVDictionary *tmp = nullptr;
    av_dict_copy(&tmp, nullptr, 0);
    if (av_opt_set_dict2(s, &tmp, AV_OPT_SEARCH_CHILDREN) < 0)
    {
        av_dict_free(&tmp);
        avformat_free_context(s);
        return devices;
    }
    avdevice_list_devices(s, &devlist);
    av_dict_free(&tmp);
    avformat_free_context(s);
    if (!devlist)
    {
        qWarning() << "avdevice_list_devices failed";
        return devices;
    }

    // Convert the list to a QVector
    devices.resize(devlist->nb_devices);
    for (int i=0; i<devlist->nb_devices; i++)
    {
        AVDeviceInfo* dev = devlist->devices[i];
        devices[i].first = dev->device_name;
        devices[i].second = dev->device_description;
    }
    avdevice_free_list_devices(&devlist);
    return devices;
}

QVector<QPair<QString, QString>> CameraDevice::getDeviceList()
{
    QVector<QPair<QString, QString>> devices;

    devices.append({"none", QObject::tr("None", "No camera device set")});

    if (!getDefaultInputFormat())
            return devices;

    if (!iformat);
#ifdef Q_OS_WIN
    else if (iformat->name == QString("dshow"))
        devices += DirectShow::getDeviceList();
#endif
#ifdef Q_OS_LINUX
    else if (iformat->name == QString("video4linux2,v4l2"))
        devices += v4l2::getDeviceList();
#endif
#ifdef Q_OS_OSX
    else if (iformat->name == QString("avfoundation"))
        devices += avfoundation::getDeviceList();
#endif
    else
        devices += getRawDeviceListGeneric();

    if (idesktopFormat)
    {
        if (idesktopFormat->name == QString("x11grab"))
            devices.push_back(QPair<QString,QString>{"x11grab#:0", QObject::tr("Desktop", "Desktop as a camera input for screen sharing")});
        if (idesktopFormat->name == QString("gdigrab"))
            devices.push_back(QPair<QString,QString>{"gdigrab#desktop", QObject::tr("Desktop", "Desktop as a camera input for screen sharing")});
    }

    return devices;
}

QString CameraDevice::getDefaultDeviceName()
{
    QString defaultdev = Settings::getInstance().getVideoDev();

    if (!getDefaultInputFormat())
        return defaultdev;

    QVector<QPair<QString, QString>> devlist = getDeviceList();
    for (const QPair<QString,QString>& device : devlist)
        if (defaultdev == device.first)
            return defaultdev;

    if (devlist.isEmpty())
        return defaultdev;

    return devlist[0].first;
}

QVector<VideoMode> CameraDevice::getVideoModes(QString devName)
{
    if (!iformat);
#ifdef Q_OS_WIN
    else if (iformat->name == QString("dshow"))
        return DirectShow::getDeviceModes(devName);
#endif
#ifdef Q_OS_LINUX
    else if (iformat->name == QString("video4linux2,v4l2"))
        return v4l2::getDeviceModes(devName);
#endif
#ifdef Q_OS_OSX
    else if (iformat->name == QString("avfoundation"))
        return avfoundation::getDeviceModes(devName);
#endif
    else
        qWarning() << "Video mode listing not implemented for input "<<iformat->name;

    (void)devName;
    return {};
}

QString CameraDevice::getPixelFormatString(uint32_t pixel_format)
{
#ifdef Q_OS_LINUX
    return v4l2::getPixelFormatString(pixel_format);
#else
    return QString("unknown");
#endif
}

bool CameraDevice::betterPixelFormat(uint32_t a, uint32_t b)
{
#ifdef Q_OS_LINUX
	return v4l2::betterPixelFormat(a, b);
#else
	return false;
#endif
}

bool CameraDevice::getDefaultInputFormat()
{
    QMutexLocker locker(&iformatLock);
    if (iformat)
        return true;

    avdevice_register_all();

    // Desktop capture input formats
#ifdef Q_OS_LINUX
    idesktopFormat = av_find_input_format("x11grab");
#endif
#ifdef Q_OS_WIN
    idesktopFormat = av_find_input_format("gdigrab");
#endif

    // Webcam input formats
#ifdef Q_OS_LINUX
    if ((iformat = av_find_input_format("v4l2")))
        return true;
#endif

#ifdef Q_OS_WIN
    if ((iformat = av_find_input_format("dshow")))
        return true;
    if ((iformat = av_find_input_format("vfwcap")))
#endif

#ifdef Q_OS_OSX
    if ((iformat = av_find_input_format("avfoundation")))
        return true;
    if ((iformat = av_find_input_format("qtkit")))
        return true;
#endif

    qWarning() << "No valid input format found";
    return false;
}
