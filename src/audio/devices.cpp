/*
    Copyright © 2014-2016 by The qTox Project

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

#include "devices.h"

#include <QDebug>
#include <QVector>

#include <RtAudio.h>

#include <QTimer>

namespace qTox {
namespace Audio {

/**
@class Device
@brief Representation of a physical audio device.
*/

/**
@class Devices
@brief Audio device manager.
*/

/**
@internal

@brief Private implementation of the Device class.

Encapsulates RtAudio::DeviceInfo and provides access to the physical audio
device.
*/
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

/**
@brief Returns a QString representation of the audio device name.
@return the device name
*/
QString Device::name() const
{
    return QString::fromStdString(d->info.name);
}

/**
The maximum input channels the audio device supports.
*/
quint32 Device::inputChannels() const
{
    return d->info.inputChannels;
}

/**
The maximum output channels the audio device supports.
*/
quint32 Device::outputChannels() const
{
    return d->info.outputChannels;
}

/**
The maximum simultaneous input/output channels the audio device supports.
*/
quint32 Device::duplexChannels() const
{
    return d->info.duplexChannels;
}

/**
Convenience method.

@return true, if outputChannels() > 0
*/
bool Device::isOutput() const
{
    return d->info.outputChannels > 0;
}

/**
@brief Returns, if this is the default audio output device.
@return true, if this is the default output device
*/
bool Device::isDefaultOutput() const
{
    return d->info.isDefaultOutput;
}

/**
Convenience method.

@return true, if inputChannels() > 0
*/
bool Device::isInput() const
{
    return d->info.inputChannels > 0;
}

/**
@brief Returns, if this is the default audio input device.
@return true, if this is the default input device
*/
bool Device::isDefaultInput() const
{
    return d->info.isDefaultOutput;
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
