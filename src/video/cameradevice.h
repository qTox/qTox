#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <QHash>
#include <QString>
#include <QMutex>
#include <QVector>
#include <atomic>

struct AVFormatContext;
struct AVInputFormat;
struct AVDeviceInfoList;

/// Maintains an FFmpeg context for open camera devices,
/// takes care of sharing the context accross users
/// and closing the camera device when not in use.
/// The device can be opened recursively,
/// and must then be closed recursively
class CameraDevice
{
public:
    /// Opens a device, creating a new one if needed
    /// Returns a nullptr if the device couldn't be opened
    static CameraDevice* open(QString devName);
    void open(); ///< Opens the device again. Never fails
    bool close(); ///< Closes the device. Never fails. If returns true, "this" becomes invalid

    /// Returns a list of device names and descriptions
    /// The names are the first part of the pair and can be passed to open(QString)
    static QVector<QPair<QString, QString> > getDeviceList();

    /// Returns the short name of the default defice
    /// This is either the device in the settings
    /// or the system default.
    static QString getDefaultDeviceName();

private:
    CameraDevice(const QString devName, AVFormatContext *context);
    static bool getDefaultInputFormat(); ///< Sets CameraDevice::iformat, returns success/failure
    static QVector<QPair<QString, QString> > getRawDeviceListGeneric(); ///< Uses avdevice_list_devices

public:
    const QString devName; ///< Short name of the device
    AVFormatContext* context; ///< Context of the open device, must always be valid

private:
    std::atomic_int refcount; ///< Number of times the device was opened
    static QHash<QString, CameraDevice*> openDevices;
    static QMutex openDeviceLock, iformatLock;
    static AVInputFormat* iformat;
};

#endif // CAMERADEVICE_H
