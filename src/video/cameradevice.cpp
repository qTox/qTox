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

QVector<QPair<QString, QString>> CameraDevice::getDeviceList()
{
    QVector<QPair<QString, QString>> devices;

    if (!getDefaultInputFormat())
        return devices;

    AVDeviceInfoList* devlist;
    avdevice_list_input_sources(iformat, nullptr, nullptr, &devlist);

    devices.resize(devlist->nb_devices);
    for (int i=0; i<devlist->nb_devices; i++)
    {
        AVDeviceInfo* dev = devlist->devices[i];
        devices[i].first = dev->device_name;
        devices[i].second = dev->device_description;
    }

    return devices;
}

QString CameraDevice::getDefaultDeviceName()
{
    QString device = Settings::getInstance().getVideoDev();

    if (!getDefaultInputFormat())
        return device;

    bool actuallyExists = false;
    AVDeviceInfoList* devlist;
    avdevice_list_input_sources(iformat, nullptr, nullptr, &devlist);

    for (int i=0; i<devlist->nb_devices; i++)
    {
        if (device == devlist->devices[i]->device_name)
        {
            actuallyExists = true;
            break;
        }
    }
    if (actuallyExists)
        return device;

    if (!devlist->nb_devices)
        return QString();

    int defaultDev = devlist->default_device == -1 ? 0 : devlist->default_device;
    return QString(devlist->devices[defaultDev]->device_name);
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
