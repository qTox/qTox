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
#include <QFile>
#include <QVector>

#include <RtAudio.h>

namespace qTox {
namespace Audio {

/**
@enum Format
@brief Describes the data format of audio buffers.
@value SINT8    8 bit signed integer
@value SINT16   16 bit signed integer
@value SINT24   24 bit signes integer
@value SINT32   32 bit signed integer
@value FLOAT32  32 bit float
@value FLOAT64  64 bit float

@fn         quint8 formatSize(Format fmt)
@brief      Returns the byte size of the @a Format type.
@param[in]  fmt     the format type

@typedef    RecordFunc
@brief      Defines an event callback, when data was recorded.

@class Device
@brief Representation of a physical audio device.

@class StreamContext
@brief Stream context.
*/

/**
This callback outputs a warning on RtAudio errors.
*/
void cb_rtaudio_error(RtAudioError::Type type, const std::string& message)
{
    Q_UNUSED(type);
    qWarning() << "RtAudio:" << QString::fromStdString(message);
}

/**
@internal
@brief Private implementation of the Device class.

Encapsulates RtAudio::DeviceInfo and provides access to the physical audio
device.
*/
class Device::Private : public QSharedData
{
    friend class Device;

public:
    Private(const RtAudio::DeviceInfo& devInfo)
        : info(devInfo)
    {
    }

private:
    RtAudio::DeviceInfo info;
};

/**
@internal
@brief Private implementation of the StreamContext class.
*/
class StreamContext::Private : public QSharedData
{
    friend class StreamContext;

public:
    /**
    This callback handles playback and recording of audio buffers.
    */
    static int cb_rtaudio_playback(void* outBuffer, void* inBuffer,
                                   unsigned int nFrames,
                                   double streamTime,
                                   RtAudioStreamStatus status,
                                   void *userData)
    {
        Q_UNUSED(streamTime);
        Q_UNUSED(status);

        const Private* p = static_cast<StreamContext::Private*>(userData);

        if (inBuffer && p->inParams)
        {
            if (p->evInput)
            {
                quint8 channels = static_cast<quint8>((*p->inParams).nChannels);
                size_t bytes = nFrames * channels * 2;
                p->evInput(inBuffer, Format::SINT16, bytes,
                           channels, p->sampleRate);
            }
        }

        if (outBuffer && p->outParams)
        {
            if (p->playbackBuffer)
                memcpy(outBuffer, p->playbackBuffer, nFrames);
        }

        return 1;
    }

public:
    Private()
        : format(RTAUDIO_SINT16)
        , sampleRate(44100)
        , opts(nullptr)
        , playthrough(false)
        , inParams(nullptr)
        , inFrames(256)
        , playbackBuffer(nullptr)
        , outParams(nullptr)
    {
        audio.showWarnings(true);
    }

    ~Private()
    {
        resetPlayback();
        delete inParams;
        delete outParams;
    }

    bool open()
    {
        if (audio.isStreamOpen())
            return true;

        if (!(inParams || outParams))
            return false;

        audio.openStream(outParams, inParams, format, sampleRate, &inFrames,
                         cb_rtaudio_playback, this, opts, cb_rtaudio_error);

        return audio.isStreamOpen();
    }

    void close()
    {
        if (audio.isStreamOpen())
            audio.closeStream();
    }

    bool abort()
    {
        resetPlayback();

        return !audio.isStreamRunning();
    }

    bool start()
    {
        if (!open())
            return false;

        audio.startStream();

        return audio.isStreamRunning();
    }

    bool stop()
    {
        if (!audio.isStreamRunning())
            return true;

        audio.stopStream();
        resetPlayback();

        return !audio.isStreamRunning();
    }

    void destroyInParams()
    {
        close();

        delete inParams;
        inParams = nullptr;
    }

    void destroyOutParams()
    {
        close();

        delete outParams;
        outParams = nullptr;
    }

    void resetPlayback()
    {
        if (audio.isStreamRunning())
            audio.abortStream();

        delete playbackBuffer;
        playbackBuffer = nullptr;
        inFrames = 0;
    }

    /**
    @internal
    @brief Restores the stream's open/running state.
    @param[in] wasOpen      the previous "open" state
    @param[in] wasRunning   the previous "running" state
    */
    void restore(bool wasOpen, bool wasRunning)
    {
        if (wasOpen)
        {
            open();

            if (wasRunning)
                start();
        }
    }

private:
    RtAudio                     audio;
    RtAudioFormat               format;
    unsigned int                sampleRate;
    RtAudio::StreamOptions*     opts;
    bool                        playthrough;

    RtAudio::StreamParameters*  inParams;
    unsigned int                inFrames;
    char*                       playbackBuffer;
    RecordFunc                  evInput;

    RtAudio::StreamParameters*  outParams;
};

/**
@brief Creates a list of all available audio input and output devices.
@return the list of available audio devices
*/
Device::List Device::availableDevices()
{
    RtAudio audio;
    List devices;

    for (unsigned int i = 0; i < audio.getDeviceCount(); i++)
    {
        devices << new Private(audio.getDeviceInfo(i));
    }

    return devices;
}

/**
@brief  Searches the available audio devices for deviceName and returns the
        index.
@param[in] deviceName   the device name
@return the device index or -1, if not found
*/
int Device::find(const QString& deviceName)
{
    RtAudio audio;
    std::string name = deviceName.toStdString();

    for (unsigned int i = 0; i < audio.getDeviceCount(); i++)
    {
        if (name == audio.getDeviceInfo(i).name)
            return static_cast<int>(i);
    }

    return -1;
}

Device::Device(Private* p)
    : d(p)
{
}

Device::Device(const Device& other)
    : d(other.d)
{
}

Device::Device(Device&& other)
    : d(std::move(other.d))
{
}

Device::~Device()
{
}

Device& Device::operator=(const Device& other)
{
    d = other.d;
    return *this;
}

Device& Device::operator=(Device&& other)
{
    d = std::move(other.d);
    return *this;
}

/**
@brief Returns the "probed" state of the audio device.
@return true when the device was successfully probed; false otherwise
*/
bool Device::isValid() const
{
    return d && d->info.probed;
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
    return d ? d->info.inputChannels : 0;
}

/**
The maximum output channels the audio device supports.
*/
quint32 Device::outputChannels() const
{
    return d ? d->info.outputChannels : 0;
}

/**
The maximum simultaneous input/output channels the audio device supports.
*/
quint32 Device::duplexChannels() const
{
    return d ? d->info.duplexChannels : 0;
}

/**
@brief Convenience method.
@return true, if outputChannels() > 0
*/
bool Device::isOutput() const
{
    return d ? d->info.outputChannels > 0 : false;
}

/**
@brief Returns, if this is the default audio output device.
@return true, if this is the default output device
*/
bool Device::isDefaultOutput() const
{
    return d ? d->info.isDefaultOutput : false;
}

/**
@brief Convenience method.
@return true, if inputChannels() > 0
*/
bool Device::isInput() const
{
    return d ? d->info.inputChannels > 0 : false;
}

/**
@brief Returns, if this is the default audio input device.
@return true, if this is the default input device
*/
bool Device::isDefaultInput() const
{
    return d ? d->info.isDefaultOutput : false;
}

/**
@brief Creates an audio stream context.
@param[in] inputDevice      the input device index; -1 == no input
@param[in] outputDevice     the output device index; -1 == no output
@return the created StreamContext or a null object, if creation failed

The created StreamContext has a maximum of 2 (stereo) channels, depending on the
devices' capabilities. A device in qTox cannot have more than two channels.
*/
StreamContext::StreamContext()
    : d(new Private())
{
}

StreamContext::StreamContext(const StreamContext& other)
    : d(other.d)
{
}

StreamContext::StreamContext(StreamContext&& other)
    : d(std::move(other.d))
{
}

StreamContext::~StreamContext()
{
}

StreamContext& StreamContext::operator=(const StreamContext& other)
{
    d = other.d;
    return *this;
}

StreamContext& StreamContext::operator=(StreamContext&& other)
{
    d = std::move(other.d);
    return *this;
}

/**
@brief Returns the StreamContext input device id.
@return the input device id or -1, if no device was assigned
*/
int StreamContext::inputDeviceId() const
{
    return (d && d->inParams) ? static_cast<int>((*d->inParams).deviceId) : -1;
}

/**
@brief Returns the StreamContext input device.
@return the input @a Device or a null @a Device, if none was assigned
*/
Device StreamContext::inputDevie() const
{
    return d && d->inParams
            ? new Device::Private(d->audio.getDeviceInfo((*d->inParams).deviceId))
            : nullptr;
}

/**
@brief Initializes the input device for the StreamContext.
@param[in] deviceId the input device id; -1 for default device
*/
void StreamContext::setInputDevice(int deviceId)
{
    if (!d)
        return;

    const bool wasOpen = d->audio.isStreamOpen();
    const bool wasRunning = d->audio.isStreamRunning();
    d->destroyInParams();

    if (deviceId < 0)
        deviceId = static_cast<int>(d->audio.getDefaultInputDevice());

    if (deviceId >= 0)
    {
        unsigned int devId = static_cast<quint32>(deviceId);
        RtAudio::DeviceInfo info = d->audio.getDeviceInfo(devId);
        d->inParams = new RtAudio::StreamParameters();
        (*d->inParams).deviceId = devId;
        (*d->inParams).nChannels = 1; //qMin<unsigned int>(info.inputChannels, 2);
        (*d->inParams).firstChannel = 0;
    }

    d->restore(wasOpen, wasRunning);
}

/**
@brief  Removes the StreamContext input device.
@note   The input device is destroyed and the stream is restored to it's
        previous open/running state.
*/
void StreamContext::removeInputDevice()
{
    if (!d)
        return;

    const bool wasOpen = d->audio.isStreamOpen();
    const bool wasRunning = d->audio.isStreamRunning();

    d->destroyInParams();
    d->restore(wasOpen, wasRunning);
}

/**
@brief Returns the StreamContext output device id.
@return the input device id or -1, if no device was assigned
*/
int StreamContext::outputDeviceId() const
{
    return (d && d->outParams) ? static_cast<int>((*d->outParams).deviceId) : -1;
}

/**
@brief Returns the StreamContext output device.
@return the output @a Device or a null @a Device, if none was assigned
*/
Device StreamContext::outputDevice() const
{

    return d && d->outParams
            ? new Device::Private(d->audio.getDeviceInfo((*d->outParams).deviceId))
            : nullptr;
}

/**
@brief Sets the output device for the StreamContext.
@param[in] deviceId     the output device id
*/
void StreamContext::setOutputDevice(int deviceId)
{
    if (!d)
        return;

    const bool wasOpen = d->audio.isStreamOpen();
    const bool wasRunning = d->audio.isStreamRunning();
    d->destroyOutParams();

    if (deviceId < 0)
        deviceId = static_cast<int>(d->audio.getDefaultOutputDevice());

    if (deviceId >= 0)
    {
        unsigned int devId = static_cast<quint32>(deviceId);
        RtAudio::DeviceInfo info = d->audio.getDeviceInfo(devId);
        d->outParams = new RtAudio::StreamParameters();
        (*d->outParams).deviceId = devId;
        (*d->outParams).nChannels = 1; //qMin<unsigned int>(info.inputChannels, 2);
        (*d->outParams).firstChannel = 0;
    }

    d->restore(wasOpen, wasRunning);
}

/**
@brief  Removes the StreamContext input device.
@note   The input device is destroyed and the stream is restored to it's
        previous open/running state.
*/
void StreamContext::removeOutputDevice()
{
    if (!d)
        return;

    const bool wasOpen = d->audio.isStreamOpen();
    const bool wasRunning = d->audio.isStreamRunning();

    d->destroyOutParams();
    d->restore(wasOpen, wasRunning);
}

/**
@brief Opens the stream for reading and writing.
@return
*/
bool StreamContext::open()
{
    return d ? d->open() : false;
}

/**
Closes the audio stream.
*/
void StreamContext::close()
{
    if (d)
        d->close();
}

/**
Aborts a stream immediately.
*/
bool StreamContext::abort()
{
    return d ? d->abort() : false;
}

/**
Starts recording or playback on the stream.
*/
bool StreamContext::start()
{
    return d ? d->start() : false;
}

/**
Waits for the streamed buffers to drain and stops playback afterwards.
*/
bool StreamContext::stop()
{
    return d ? d->stop() : false;
}

void StreamContext::playback(char* pcm, Format fmt, quint32 frames,
                             quint8 channels, quint32 sampleRate)
{
    // TODO: map the number of channels (currently assuming input == output)
    Q_UNUSED(channels);

    if (!d)
        return;

    if (sampleRate != d->sampleRate)
    {
        d->close();
        d->sampleRate = sampleRate;
    }
    else
    {
        d->resetPlayback();
    }

    d->format = static_cast<RtAudioFormat>(fmt);
    d->inFrames = frames;
    d->playbackBuffer = pcm;

    d->start();
}

void StreamContext::playback(const QString& fileName)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly))
    {
        char* data = new char[f.size()];
        f.read(data, f.size());
        Format fmt = Format::SINT16;
        quint32 frames = static_cast<quint32>(f.size()) / formatSize(fmt);

        playback(data, fmt, frames, 1, 44100);
    }
    else
    {
        qWarning() << "Could not open audio file for playback:" << fileName;
    }
}

bool StreamContext::isOpen() const
{
    return d ? d->audio.isStreamOpen() : false;
}

bool StreamContext::isRunning() const
{
    return d ? d->audio.isStreamRunning() : false;
}

void StreamContext::onRecorded(RecordFunc event)
{
    if (!d)
        return;

    d->evInput = event;
}

}
}
