#include <QDebug>
extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
}
#include "cameradevice.h"
#include "src/misc/settings.h"

#ifdef Q_OS_WIN
#include "src/platform/camera/directshow.h"
#endif

QHash<QString, CameraDevice*> CameraDevice::openDevices;
QMutex CameraDevice::openDeviceLock, CameraDevice::iformatLock;
AVInputFormat* CameraDevice::iformat{nullptr};

CameraDevice::CameraDevice(const QString devName, AVFormatContext *context)
    : devName{devName}, context{context}, refcount{1}
{
}

CameraDevice* CameraDevice::open(QString devName, AVDictionary** options)
{
    openDeviceLock.lock();
    AVFormatContext* fctx = nullptr;
    CameraDevice* dev = openDevices.value(devName);
    if (dev)
        goto out;

    if (avformat_open_input(&fctx, devName.toStdString().c_str(), iformat, options)<0)
        goto out;

    if (avformat_find_stream_info(fctx, NULL) < 0)
    {
        avformat_close_input(&fctx);
        goto out;
    }

    dev = new CameraDevice{devName, fctx};
    openDevices[devName] = dev;

out:
    openDeviceLock.unlock();
    return dev;
}

CameraDevice* CameraDevice::open(QString devName)
{
    return open(devName, nullptr);
}

CameraDevice* CameraDevice::open(QString devName, VideoMode mode)
{
    if (!getDefaultInputFormat())
        return nullptr;

    AVDictionary* options = nullptr;
    if (false);
#ifdef Q_OS_WIN
    else if (iformat->name == QString("dshow"))
    {
        av_dict_set(&options, "video_size", QString("%1x%2").arg(mode.width).arg(mode.height).toStdString().c_str(), 0);
        av_dict_set(&options, "framerate", QString().setNum(mode.FPS).toStdString().c_str(), 0);
    }
#endif
    else
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
    }
    avdevice_list_devices(s, &devlist);
    if (!devlist)
        qWarning() << "avdevice_list_devices failed";

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
    if (!getDefaultInputFormat())
            return {};

    if (false);
#ifdef Q_OS_WIN
    else if (iformat->name == QString("dshow"))
        return DirectShow::getDeviceList();
#endif
    else
        return getRawDeviceListGeneric();
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
    if (false);
#ifdef Q_OS_WIN
    else if (iformat->name == QString("dshow"))
        return DirectShow::getDeviceModes(devName);
#endif
    else
        qWarning() << "Video mode listing not implemented for input "<<iformat->name;

    (void)devName;
    return {};
}

bool CameraDevice::getDefaultInputFormat()
{
    QMutexLocker locker(&iformatLock);
    if (iformat)
        return true;

    avdevice_register_all();

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
