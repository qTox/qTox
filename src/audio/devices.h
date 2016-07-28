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

#ifndef QTOX_AUDIO_H
#define QTOX_AUDIO_H

#include <functional>

#include <QObject>
#include <QSharedData>

namespace qTox {
namespace Audio {

enum class Format
{
    SINT8   = 0x01,
    SINT16  = 0x02,
    SINT24  = 0x04,
    SINT32  = 0x08,
    FLOAT32 = 0x10,
    FLOAT64 = 0x20
};

static inline quint8 formatSize(Format fmt)
{
    switch (fmt)
    {
    case Format::SINT8: return 1;
    case Format::SINT16: return 2;
    case Format::SINT24: return 3;
    case Format::SINT32:
    case Format::FLOAT32: return 4;
    case Format::FLOAT64: return 8;
    }
}

class Device
{
    class Private;

    friend class StreamContext;

public:
    typedef QVector<Device> List;

public:
    static List availableDevices();
    static int find(const QString& name);

public:
    Device(Private* p = nullptr);
    Device(const Device& other);
    Device(Device&& other);
    ~Device();

    Device& operator=(const Device& other);
    Device& operator=(Device&& other);

public:
    inline bool isNull() const
    {
        return !d;
    }

    bool isValid() const;
    QString name() const;
    quint32 inputChannels() const;
    quint32 outputChannels() const;
    quint32 duplexChannels() const;
    bool isOutput() const;
    bool isDefaultOutput() const;
    bool isInput() const;
    bool isDefaultInput() const;

private:
    QExplicitlySharedDataPointer<Private> d;
};

class StreamContext
{
public:
    class Private;

public:
    StreamContext();
    StreamContext(const StreamContext& other);
    StreamContext(StreamContext&& other);
    ~StreamContext();

    StreamContext& operator=(const StreamContext& other);
    StreamContext& operator=(StreamContext&& other);

    inline bool isNull() const
    {
        return !d;
    }

    int inputDeviceId() const;
    Device inputDevie() const;
    void setInputDevice(int deviceId = -1);
    void removeInputDevice();

    int outputDeviceId() const;
    Device outputDevice() const;
    void setOutputDevice(int deviceId = -1);
    void removeOutputDevice();

    bool open();
    void close();

    bool abort();
    bool start();
    bool stop();

    bool isOpen() const;
    bool isRunning() const;

private:
    QExplicitlySharedDataPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(qTox::Audio::Device)

#endif
