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

class Device
{
    class Private;

public:
    typedef QVector<Device> List;

public:
    static List availableDevices();
    static int find(const QString& name);

public:
    Device();
    Device(Private* p);
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
    class Private;

public:
    explicit StreamContext(int inputDevice = -1, int outputDevice = -1);
    StreamContext(Private* p);
    StreamContext(const StreamContext& other);
    StreamContext(StreamContext&& other);
    ~StreamContext();

    StreamContext& operator=(const StreamContext& other);
    StreamContext& operator=(StreamContext&& other);

    inline bool isNull() const
    {
        return !d;
    }

    bool open();
    void close();

    void start();
    void stop();

    bool isOpen() const;
    bool isRunning() const;

private:
    QExplicitlySharedDataPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(qTox::Audio::Device)

#endif
