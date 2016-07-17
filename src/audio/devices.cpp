#include "devices.h"

#include <QDebug>
#include <QVector>

#include <RtAudio.h>

#include <QTimer>

namespace qTox {
namespace Audio {

class Device::Private : public QSharedData
{
public:
    Private(const RtAudio::DeviceInfo& devInfo)
        : info(devInfo)
    {
    }

public:
    RtAudio::DeviceInfo info;
};

class Devices::Private
{
public:
    Private() = default;

    int count()
    {
        return static_cast<int>(audio.getDeviceCount());
    }

public:
    RtAudio audio;

    QVector<Device::PrivatePtr> activeDevices;
};

Device::Device(Private* dev)
    : d(dev)
{
}

Device& Device::operator=(Device& other)
{
    d = other.d;
    return *this;
}

bool Device::isValid() const
{
    return d->info.probed;
}

QString Device::name() const
{
    return QString::fromStdString(d->info.name);
}

/**
Returns the singleton instance.
*/
Devices& Devices::self()
{
    static Devices instance;
    return instance;
}

Devices::Devices()
    : d(new Private)
{
}

Devices::~Devices()
{
    delete d;
}

}
}
