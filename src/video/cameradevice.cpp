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

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#pragma GCC diagnostic pop
}
#include "cameradevice.h"
#include "src/persistence/settings.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#define USING_V4L 1
#else
#define USING_V4L 0
#endif

#ifdef Q_OS_WIN
#include "src/platform/camera/directshow.h"
#endif
#if USING_V4L
#include "src/platform/camera/v4l2.h"
#endif
#ifdef Q_OS_OSX
#include "src/platform/camera/avfoundation.h"
#endif

/**
 * @class CameraDevice
 *
 * Maintains an FFmpeg context for open camera devices,
 * takes care of sharing the context accross users and closing
 * the camera device when not in use. The device can be opened
 * recursively, and must then be closed recursively
 */


/**
 * @var const QString CameraDevice::devName
 * @brief Short name of the device
 *
 * @var AVFormatContext* CameraDevice::context
 * @brief Context of the open device, must always be valid
 *
 * @var std::atomic_int CameraDevice::refcount;
 * @brief Number of times the device was opened
 */


QHash<QString, CameraDevice*> CameraDevice::openDevices;
QMutex CameraDevice::openDeviceLock, CameraDevice::iformatLock;
AVInputFormat* CameraDevice::iformat{nullptr};
AVInputFormat* CameraDevice::idesktopFormat{nullptr};

CameraDevice::CameraDevice(const QString& devName, AVFormatContext* context)
    : devName{devName}
    , context{context}
    , refcount{1}
{
}

CameraDevice* CameraDevice::open(QString devName, AVDictionary** options)
{
    openDeviceLock.lock();
    AVFormatContext* fctx = nullptr;
    CameraDevice* dev = openDevices.value(devName);
    int aduration;
    std::string devString;
    if (dev) {
        goto out;
    }

    AVInputFormat* format;
    if (devName.startsWith("x11grab#")) {
        devName = devName.mid(8);
        format = idesktopFormat;
    } else if (devName.startsWith("gdigrab#")) {
        devName = devName.mid(8);
        format = idesktopFormat;
    } else {
        format = iformat;
    }

    devString = devName.toStdString();
    if (avformat_open_input(&fctx, devString.c_str(), format, options) < 0) {
        goto out;
    }

// Fix avformat_find_stream_info hanging on garbage input
#if FF_API_PROBESIZE_32
    aduration = fctx->max_analyze_duration2 = 0;
#else
    aduration = fctx->max_analyze_duration = 0;
#endif

    if (avformat_find_stream_info(fctx, nullptr) < 0) {
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

/**
 * @brief Opens a device.
 *
 * Opens a device, creating a new one if needed
 * If the device is alreay open in another mode, the mode
 * will be ignored and the existing device is used
 * If the mode does not exist, a new device can't be opened.
 *
 * @param devName Device name to open.
 * @param mode Mode of device to open.
 * @return CameraDevice if the device could be opened, nullptr otherwise.
 */
CameraDevice* CameraDevice::open(QString devName, VideoMode mode)
{
    if (!getDefaultInputFormat())
        return nullptr;

    if (devName == "none") {
        qDebug() << "Tried to open the null device";
        return nullptr;
    }

    float FPS = 5;
    if (mode.FPS > 0.0f) {
        FPS = mode.FPS;
    } else {
        qWarning() << "VideoMode could be invalid!";
    }

    const std::string videoSize = QStringLiteral("%1x%2").arg(mode.width).arg(mode.height).toStdString();
    const std::string framerate = QString{}.setNum(FPS).toStdString();

    AVDictionary* options = nullptr;
    if (!iformat)
        ;
#if USING_V4L
    else if (devName.startsWith("x11grab#")) {
        QSize screen;
        if (mode.width && mode.height) {
            screen.setWidth(mode.width);
            screen.setHeight(mode.height);
        } else {
            QScreen* defaultScreen = QApplication::primaryScreen();
            qreal pixRatio = defaultScreen->devicePixelRatio();

            screen = defaultScreen->size();
            // Workaround https://trac.ffmpeg.org/ticket/4574 by choping 1 px bottom and right
            // Actually, let's chop two pixels, toxav hates odd resolutions (off by one stride)
            screen.setWidth((screen.width() * pixRatio) - 2);
            screen.setHeight((screen.height() * pixRatio) - 2);
        }
        const std::string screenVideoSize = QStringLiteral("%1x%2").arg(screen.width()).arg(screen.height()).toStdString();
        av_dict_set(&options, "video_size", screenVideoSize.c_str(), 0);
        devName += QString("+%1,%2").arg(QString().setNum(mode.x), QString().setNum(mode.y));

        av_dict_set(&options, "framerate", framerate.c_str(), 0);
    } else if (iformat->name == QString("video4linux2,v4l2") && mode) {
        av_dict_set(&options, "video_size", videoSize.c_str(), 0);
        av_dict_set(&options, "framerate", framerate.c_str(), 0);
        const std::string pixelFormatStr = v4l2::getPixelFormatString(mode.pixel_format).toStdString();
        // don't try to set a format string that doesn't exist
        if (pixelFormatStr != "unknown" && pixelFormatStr != "invalid") {
            const char* pixel_format = pixelFormatStr.c_str();
            av_dict_set(&options, "pixel_format", pixel_format, 0);
        }
    }
#endif
#ifdef Q_OS_WIN
    else if (devName.startsWith("gdigrab#")) {

        const std::string offsetX = QString().setNum(mode.x).toStdString();
        const std::string offsetY = QString().setNum(mode.y).toStdString();
        av_dict_set(&options, "framerate", framerate.c_str(), 0);
        av_dict_set(&options, "offset_x", offsetX.c_str(), 0);
        av_dict_set(&options, "offset_y", offsetY.c_str(), 0);
        av_dict_set(&options, "video_size", videoSize.c_str(), 0);
    } else if (iformat->name == QString("dshow") && mode) {
        av_dict_set(&options, "video_size", videoSize.c_str(), 0);
        av_dict_set(&options, "framerate", framerate.c_str(), 0);
    }
#endif
#ifdef Q_OS_OSX
    else if (iformat->name == QString("avfoundation")) {
        if (mode) {
            av_dict_set(&options, "video_size", videoSize.c_str(), 0);
            av_dict_set(&options, "framerate", framerate.c_str(), 0);
        } else if (devName.startsWith(avfoundation::CAPTURE_SCREEN)) {
            av_dict_set(&options, "framerate", framerate.c_str(), 0);
            av_dict_set_int(&options, "capture_cursor", 1, 0);
            av_dict_set_int(&options, "capture_mouse_clicks", 1, 0);
        }
    }
#endif
    else if (mode) {
        qWarning().nospace() << "No known options for " << iformat->name << ", using defaults.";
        Q_UNUSED(mode);
    }

    CameraDevice* dev = open(devName, &options);
    if (options) {
        av_dict_free(&options);
    }

    return dev;
}

/**
 * @brief Opens the device again. Never fails
 */
void CameraDevice::open()
{
    ++refcount;
}

/**
 * @brief Closes the device. Never fails.
 * @note If returns true, "this" becomes invalid.
 * @return True, if device finally deleted (closed last reference),
 * false otherwise (if other references exist).
 */
bool CameraDevice::close()
{
    if (--refcount > 0)
        return false;

    openDeviceLock.lock();
    openDevices.remove(devName);
    openDeviceLock.unlock();
    avformat_close_input(&context);
    delete this;
    return true;
}

/**
 * @brief Get raw device list
 * @note Uses avdevice_list_devices
 * @return Raw device list
 */
QVector<QPair<QString, QString>> CameraDevice::getRawDeviceListGeneric()
{
    QVector<QPair<QString, QString>> devices;

    if (!getDefaultInputFormat())
        return devices;

    // Alloc an input device context
    AVFormatContext* s;
    if (!(s = avformat_alloc_context()))
        return devices;

    if (!iformat->priv_class || !AV_IS_INPUT_DEVICE(iformat->priv_class->category)) {
        avformat_free_context(s);
        return devices;
    }

    s->iformat = iformat;
    if (s->iformat->priv_data_size > 0) {
        s->priv_data = av_mallocz(s->iformat->priv_data_size);
        if (!s->priv_data) {
            avformat_free_context(s);
            return devices;
        }
        if (s->iformat->priv_class) {
            *static_cast<const AVClass**>(s->priv_data) = s->iformat->priv_class;
            av_opt_set_defaults(s->priv_data);
        }
    } else {
        s->priv_data = nullptr;
    }

    // List the devices for this context
    AVDeviceInfoList* devlist = nullptr;
    AVDictionary* tmp = nullptr;
    av_dict_copy(&tmp, nullptr, 0);
    if (av_opt_set_dict2(s, &tmp, AV_OPT_SEARCH_CHILDREN) < 0) {
        av_dict_free(&tmp);
        avformat_free_context(s);
        return devices;
    }
    avdevice_list_devices(s, &devlist);
    av_dict_free(&tmp);
    avformat_free_context(s);
    if (!devlist) {
        qWarning() << "avdevice_list_devices failed";
        return devices;
    }

    // Convert the list to a QVector
    devices.resize(devlist->nb_devices);
    for (int i = 0; i < devlist->nb_devices; ++i) {
        AVDeviceInfo* dev = devlist->devices[i];
        devices[i].first = dev->device_name;
        devices[i].second = dev->device_description;
    }
    avdevice_free_list_devices(&devlist);
    return devices;
}

/**
 * @brief Get device list with desciption
 * @return A list of device names and descriptions.
 * The names are the first part of the pair and can be passed to open(QString).
 */
QVector<QPair<QString, QString>> CameraDevice::getDeviceList()
{
    QVector<QPair<QString, QString>> devices;

    devices.append({"none", QObject::tr("None", "No camera device set")});

    if (!getDefaultInputFormat())
        return devices;

    if (!iformat)
        ;
#ifdef Q_OS_WIN
    else if (iformat->name == QString("dshow"))
        devices += DirectShow::getDeviceList();
#endif
#if USING_V4L
    else if (iformat->name == QString("video4linux2,v4l2"))
        devices += v4l2::getDeviceList();
#endif
#ifdef Q_OS_OSX
    else if (iformat->name == QString("avfoundation"))
        devices += avfoundation::getDeviceList();
#endif
    else
        devices += getRawDeviceListGeneric();

    if (idesktopFormat) {
        if (idesktopFormat->name == QString("x11grab")) {
            QString dev = "x11grab#";
            QByteArray display = qgetenv("DISPLAY");

            if (display.isNull())
                dev += ":0";
            else
                dev += display.constData();

            devices.push_back(QPair<QString, QString>{
                dev, QObject::tr("Desktop", "Desktop as a camera input for screen sharing")});
        }
        if (idesktopFormat->name == QString("gdigrab"))
            devices.push_back(QPair<QString, QString>{
                "gdigrab#desktop",
                QObject::tr("Desktop", "Desktop as a camera input for screen sharing")});
    }

    return devices;
}

/**
 * @brief Get the default device name.
 * @return The short name of the default device
 * This is either the device in the settings or the system default.
 */
QString CameraDevice::getDefaultDeviceName()
{
    QString defaultdev = Settings::getInstance().getVideoDev();

    if (!getDefaultInputFormat())
        return defaultdev;

    QVector<QPair<QString, QString>> devlist = getDeviceList();
    for (const QPair<QString, QString>& device : devlist)
        if (defaultdev == device.first)
            return defaultdev;

    if (devlist.isEmpty())
        return defaultdev;

    return devlist[0].first;
}

/**
 * @brief Checks if a device name specifies a display.
 * @param devName Device name to check.
 * @return True, if device is screen, false otherwise.
 */
bool CameraDevice::isScreen(const QString& devName)
{
    return devName.startsWith("x11grab") || devName.startsWith("gdigrab");
}

/**
 * @brief Get list of resolutions and position of screens
 * @return Vector of avaliable screen modes with offset
 */
QVector<VideoMode> CameraDevice::getScreenModes()
{
    QList<QScreen*> screens = QApplication::screens();
    QVector<VideoMode> result;

    std::for_each(screens.begin(), screens.end(), [&result](QScreen* s) {
        QRect rect = s->geometry();
        QPoint p = rect.topLeft();
        qreal pixRatio = s->devicePixelRatio();

        VideoMode mode(rect.width() * pixRatio, rect.height() * pixRatio, p.x() * pixRatio,
                       p.y() * pixRatio);
        result.push_back(mode);
    });

    return result;
}

/**
 * @brief Get the list of video modes for a device.
 * @param devName Device name to get nodes from.
 * @return Vector of available modes for the device.
 */
QVector<VideoMode> CameraDevice::getVideoModes(QString devName)
{
    Q_UNUSED(devName);

    if (!iformat)
        ;
    else if (isScreen(devName))
        return getScreenModes();
#ifdef Q_OS_WIN
    else if (iformat->name == QString("dshow"))
        return DirectShow::getDeviceModes(devName);
#endif
#if USING_V4L
    else if (iformat->name == QString("video4linux2,v4l2"))
        return v4l2::getDeviceModes(devName);
#endif
#ifdef Q_OS_OSX
    else if (iformat->name == QString("avfoundation"))
        return avfoundation::getDeviceModes(devName);
#endif
    else
        qWarning() << "Video mode listing not implemented for input " << iformat->name;

    return {};
}

/**
 * @brief Get the name of the pixel format of a video mode.
 * @param pixel_format Pixel format to get the name from.
 * @return Name of the pixel format.
 */
QString CameraDevice::getPixelFormatString(uint32_t pixel_format)
{
#if USING_V4L
    return v4l2::getPixelFormatString(pixel_format);
#else
    return QString("unknown");
#endif
}

/**
 * @brief Compare two pixel formats.
 * @param a First pixel format to compare.
 * @param b Second pixel format to compare.
 * @return True if we prefer format a to b,
 * false otherwise (such as if there's no preference).
 */
bool CameraDevice::betterPixelFormat(uint32_t a, uint32_t b)
{
#if USING_V4L
    return v4l2::betterPixelFormat(a, b);
#else
    return false;
#endif
}

/**
 * @brief Sets CameraDevice::iformat to default.
 * @return True if success, false if failure.
 */
bool CameraDevice::getDefaultInputFormat()
{
    QMutexLocker locker(&iformatLock);
    if (iformat)
        return true;

    avdevice_register_all();

// Desktop capture input formats
#if USING_V4L
    idesktopFormat = av_find_input_format("x11grab");
#endif
#ifdef Q_OS_WIN
    idesktopFormat = av_find_input_format("gdigrab");
#endif

// Webcam input formats
#if USING_V4L
    if ((iformat = av_find_input_format("v4l2")))
        return true;
#endif

#ifdef Q_OS_WIN
    if ((iformat = av_find_input_format("dshow")))
        return true;
    if ((iformat = av_find_input_format("vfwcap")))
        return true;
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
