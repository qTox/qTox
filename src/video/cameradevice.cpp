#include <QDebug>
extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
}
#include "cameradevice.h"
#include "src/misc/settings.h"

QHash<QString, CameraDevice*> CameraDevice::openDevices;
QMutex CameraDevice::openDeviceLock, CameraDevice::iformatLock;
AVInputFormat* CameraDevice::iformat{nullptr};

CameraDevice::CameraDevice(const QString devName, AVFormatContext *context)
    : devName{devName}, context{context}, refcount{1}
{

}

CameraDevice* CameraDevice::open(QString devName)
{
    openDeviceLock.lock();
    AVFormatContext* fctx = nullptr;
    CameraDevice* dev = openDevices.value(devName);
    if (dev)
        goto out;

    if (avformat_open_input(&fctx, devName.toStdString().c_str(), nullptr, nullptr)<0)
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

AVDeviceInfoList* CameraDevice::getRawDeviceList()
{
    AVDeviceInfoList* devlist = nullptr;

    if (!getDefaultInputFormat())
        return devlist;

    // Alloc an input device context
    AVFormatContext *s;
    if (!(s = avformat_alloc_context()))
        return devlist;
    if (!iformat->priv_class || !AV_IS_INPUT_DEVICE(iformat->priv_class->category))
    {
        avformat_free_context(s);
        return devlist;
    }
    s->iformat = iformat;
    if (s->iformat->priv_data_size > 0)
    {
        s->priv_data = av_mallocz(s->iformat->priv_data_size);
        if (!s->priv_data)
        {
            avformat_free_context(s);
            return devlist;
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
    AVDictionary *tmp = nullptr;
    av_dict_copy(&tmp, nullptr, 0);
    if (av_opt_set_dict2(s, &tmp, AV_OPT_SEARCH_CHILDREN) < 0)
    {
        av_dict_free(&tmp);
        avformat_free_context(s);
    }
    avdevice_list_devices(s, &devlist);
    return devlist;
}

QVector<QPair<QString, QString>> CameraDevice::getDeviceList()
{
    QVector<QPair<QString, QString>> devices;

    AVDeviceInfoList* devlist = getRawDeviceList();
    if (!devlist)
        return devices;

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

QString CameraDevice::getDefaultDeviceName()
{
    QString device = Settings::getInstance().getVideoDev();

    if (!getDefaultInputFormat())
        return device;

    AVDeviceInfoList* devlist = getRawDeviceList();
    if (!devlist)
        return device;

    bool actuallyExists = false;
    for (int i=0; i<devlist->nb_devices; i++)
    {
        if (device == devlist->devices[i]->device_name)
        {
            actuallyExists = true;
            break;
        }
    }
    if (actuallyExists)
    {
        avdevice_free_list_devices(&devlist);
        return device;
    }

    if (!devlist->nb_devices)
    {
        device.clear();
    }
    else
    {
        int defaultDev = devlist->default_device == -1 ? 0 : devlist->default_device;
        device = QString(devlist->devices[defaultDev]->device_name);
    }
    avdevice_free_list_devices(&devlist);
    return device;
}

bool CameraDevice::getDefaultInputFormat()
{
    QMutexLocker locker(&iformatLock);
    if (iformat)
        return true;

    avdevice_register_all();

    // Linux
    if ((iformat = av_find_input_format("v4l2")))
        return true;

    // Windows
    if ((iformat = av_find_input_format("dshow")))
        return true;

    // Mac
    if ((iformat = av_find_input_format("avfoundation")))
        return true;
    if ((iformat = av_find_input_format("qtkit")))
        return true;

    qWarning() << "No valid input format found";
    return false;
}
