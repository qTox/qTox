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

QSet<const Audio::Device::Private*> Audio::activeDevices = QSet<const Audio::Device::Private*>();
QSet<const Audio::Stream::Private*> Audio::streams = QSet<const Audio::Stream::Private*>();

/**
@class Audio
@brief A non-constructable wrapper class to qTox audio features.


@enum Audio::Format
@brief Describes the data format of audio buffers.
@value SINT8    8 bit signed integer
@value SINT16   16 bit signed integer
@value SINT24   24 bit signes integer
@value SINT32   32 bit signed integer
@value FLOAT32  32 bit float
@value FLOAT64  64 bit float


@fn         quint8 formatSize(Audio::Format fmt)
@brief      Returns the byte size of the @a Format type.
@param[in]  fmt     the format type


@typedef    Audio::RecordFunc
@brief      Event callback, when data was recorded.
@param[in]  pcm     the PCM encoded audio data
@param[in]  fmt     the  buffer's audio format type
@param[in]  frames  the buffer's number of frames
@param[in]  channels    the buffer's number of channels
@param[in]  sampleRate  the buffer's sample rate


@class  Audio::Device
@brief  Representation of a physical audio device.


@class  Audio::Stream
@brief  Defines the streaming context.

Manages input and/or output audio resources in a streaming context. Input can
either be a physical audio device or a buffer. If a physical input device is
configured, it can forward its' data to the configured output (playthrough).
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
 * @internal
 * @brief audio resource management
 */
class Audio::Private
{
public:
    static int streamCount()
    {
        return Audio::streams.count();
    }

    static void insert(const Stream::Private* stream)
    {
        if (stream)
        {
            Audio::streams << stream;
            qDebug() << "Registered audio stream. Streams:" << streams.count();
        }
    }

    static void remove(const Stream::Private* stream)
    {
        if (Audio::streams.remove(stream))
            qDebug() << "Removed audio stream. Streams:" << streams.count();
    }

    static int openDevices()
    {
        return Audio::activeDevices.count();
    }

    static void insert(const Device::Private* device)
    {
        if (device)
        {
            Audio::activeDevices << device;
            qDebug() << "Registered open audio device. Active devices:" << activeDevices.count();
        }
    }

    static void remove(const Device::Private* device)
    {
        if (Audio::activeDevices.remove(device))
            qDebug() << "Removed open audio device. Active devices:" << activeDevices.count();
    }
};

/**
@internal
@brief Private implementation of the Device class.

Encapsulates RtAudio::DeviceInfo and provides access to the physical audio
device.
*/
class Audio::Device::Private : public QSharedData
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
@brief Private implementation of the Stream class.
*/
class Audio::Stream::Private : public QSharedData
{
    friend class Stream;

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

        int ret = 1;
        const Private* p = static_cast<Stream::Private*>(userData);

        if (p->inParams && nFrames > 0)
        {
            if (p->evInput)
            {
                quint8 channels = static_cast<quint8>((*p->inParams).nChannels);
                ret = p->evInput(inBuffer, Format::SINT16, nFrames,
                                 channels, p->sampleRate);
            }
        }

        if (p->outParams)
        {
            if (p->playbackBuffer)
            {
                memcpy(outBuffer, p->playbackBuffer, nFrames);
            }
            else if (p->playthrough)
            {
                quint8 channels = static_cast<quint8>((*p->inParams).nChannels);
                size_t bytes = nFrames * channels * 2;
                memcpy(outBuffer, inBuffer, bytes);
            }
        }

        return ret;
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
        Audio::Private::insert(this);
        audio.showWarnings(true);
    }

    ~Private()
    {
        Audio::Private::remove(this);
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

        delete[] playbackBuffer;
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
Audio::Device::List Audio::availableDevices()
{
    RtAudio audio;
    Device::List devices;

    for (unsigned int i = 0; i < audio.getDeviceCount(); i++)
    {
        devices << new Device::Private(audio.getDeviceInfo(i));
    }

    return devices;
}

/**
@brief  Searches the available audio devices for deviceName and returns the
        index.
@param[in] deviceName   the device name
@return the device index or -1, if not found
*/
int Audio::Device::find(const QString& deviceName)
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

Audio::Device::Device(Private* p)
    : d(p)
{
}

Audio::Device::Device(const Device& other)
    : d(other.d)
{
}

Audio::Device::Device(Device&& other)
    : d(std::move(other.d))
{
}

Audio::Device::~Device()
{
}

Audio::Device& Audio::Device::operator=(const Device& other)
{
    d = other.d;
    return *this;
}

Audio::Device& Audio::Device::operator=(Device&& other)
{
    d = std::move(other.d);
    return *this;
}

/**
@brief Returns the "probed" state of the audio device.
@return true when the device was successfully probed; false otherwise
*/
bool Audio::Device::isValid() const
{
    return d && d->info.probed;
}

/**
@brief Returns a QString representation of the audio device name.
@return the device name
*/
QString Audio::Device::name() const
{
    return QString::fromStdString(d->info.name);
}

/**
The maximum input channels the audio device supports.
*/
quint32 Audio::Device::inputChannels() const
{
    return d ? d->info.inputChannels : 0;
}

/**
The maximum output channels the audio device supports.
*/
quint32 Audio::Device::outputChannels() const
{
    return d ? d->info.outputChannels : 0;
}

/**
The maximum simultaneous input/output channels the audio device supports.
*/
quint32 Audio::Device::duplexChannels() const
{
    return d ? d->info.duplexChannels : 0;
}

/**
@brief Convenience method.
@return true, if outputChannels() > 0
*/
bool Audio::Device::isOutput() const
{
    return d ? d->info.outputChannels > 0 : false;
}

/**
@brief Returns, if this is the default audio output device.
@return true, if this is the default output device
*/
bool Audio::Device::isDefaultOutput() const
{
    return d ? d->info.isDefaultOutput : false;
}

/**
@brief Convenience method.
@return true, if inputChannels() > 0
*/
bool Audio::Device::isInput() const
{
    return d ? d->info.inputChannels > 0 : false;
}

/**
@brief Returns, if this is the default audio input device.
@return true, if this is the default input device
*/
bool Audio::Device::isDefaultInput() const
{
    return d ? d->info.isDefaultOutput : false;
}

/**
@brief Creates an audio stream context.
@param[in] inputDevice      the input device index; -1 == no input
@param[in] outputDevice     the output device index; -1 == no output
@return the created Stream or a null object, if creation failed

The created Stream has a maximum of 2 (stereo) channels, depending on the
devices' capabilities. A device in qTox cannot have more than two channels.
*/
Audio::Stream::Stream()
    : d(new Private())
{
}

Audio::Stream::Stream(const Stream& other)
    : d(other.d)
{
}

Audio::Stream::Stream(Stream&& other)
    : d(std::move(other.d))
{
}

Audio::Stream::~Stream()
{
}

Audio::Stream& Audio::Stream::operator=(const Stream& other)
{
    d = other.d;
    return *this;
}

Audio::Stream& Audio::Stream::operator=(Stream&& other)
{
    d = std::move(other.d);
    return *this;
}

/**
@brief  The current sample rate.
@return the sample rate in Hz.
*/
quint32 Audio::Stream::sampleRate() const
{
    return d ? d->sampleRate : 0;
}

/**
@brief Sets the sample rate for the stream context.

@note An open stream will be closed by this method.
*/
void Audio::Stream::setSampleRate(quint32 hz)
{
    if (!d)
        return;

    if (hz != d->sampleRate)
    {
        d->close();
        d->sampleRate = hz;
    }
}

/**
@brief Returns the current frame count.
@return The Stream's (input) frame count.
*/
quint32 Audio::Stream::frameCount() const
{
    return d ? d->inFrames : 0;
}

/**
@brief  Sets the frame count (for internal frame buffer) for input.
@note   Can only be set for an input or playthrough (duplex) Stream.
*/
void Audio::Stream::setFrameCount(quint32 frames)
{
    if (!d || frames == d->inFrames)
        return;

    Q_ASSERT_X(d->inParams, __func__,
               "Frame count can only be set within an input context.");

    Q_ASSERT_X(!d->audio.isStreamRunning(), __func__,
               "Frame count cannot be set on an active stream.");

    d->inFrames = frames;
}

/**
@brief Returns the Stream input device id.
@return the input device id or -1, if no device was assigned
*/
int Audio::Stream::inputDeviceId() const
{
    return (d && d->inParams) ? static_cast<int>((*d->inParams).deviceId) : -1;
}

/**
@brief Returns the Stream input device.
@return the input @a Device or a null @a Device, if none was assigned
*/
Audio::Device Audio::Stream::inputDevie() const
{
    return d && d->inParams
            ? new Device::Private(d->audio.getDeviceInfo((*d->inParams).deviceId))
            : nullptr;
}

/**
@brief Initializes the input device for the Stream.
@param[in] deviceId the input device id; -1 for default device
*/
void Audio::Stream::setInputDevice(int deviceId)
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
        (*d->inParams).nChannels = qMin<unsigned int>(info.inputChannels, 2);
        (*d->inParams).firstChannel = 0;
    }

    d->restore(wasOpen, wasRunning);
}

/**
@brief  Removes the Stream input device.
@note   The input device is destroyed and the stream is restored to it's
        previous open/running state.
*/
void Audio::Stream::removeInputDevice()
{
    if (!d)
        return;

    const bool wasOpen = d->audio.isStreamOpen();
    const bool wasRunning = d->audio.isStreamRunning();

    d->destroyInParams();
    d->restore(wasOpen, wasRunning);
}

/**
@brief Returns the Stream output device id.
@return the input device id or -1, if no device was assigned
*/
int Audio::Stream::outputDeviceId() const
{
    return (d && d->outParams) ? static_cast<int>((*d->outParams).deviceId) : -1;
}

/**
@brief Returns the Stream output device.
@return the output @a Device or a null @a Device, if none was assigned
*/
Audio::Device Audio::Stream::outputDevice() const
{

    return d && d->outParams
            ? new Device::Private(d->audio.getDeviceInfo((*d->outParams).deviceId))
            : nullptr;
}

/**
@brief Sets the output device for the Stream.
@param[in] deviceId     the output device id
*/
void Audio::Stream::setOutputDevice(int deviceId)
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
        (*d->outParams).nChannels = qMin<unsigned int>(info.inputChannels, 2);
        (*d->outParams).firstChannel = 0;
    }

    d->restore(wasOpen, wasRunning);
}

/**
@brief  Removes the Stream input device.
@note   The input device is destroyed and the stream is restored to it's
        previous open/running state.
*/
void Audio::Stream::removeOutputDevice()
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
bool Audio::Stream::open()
{
    return d ? d->open() : false;
}

/**
Closes the audio stream.
*/
void Audio::Stream::close()
{
    if (d)
        d->close();
}

/**
Aborts a stream immediately.
*/
bool Audio::Stream::abort()
{
    return d ? d->abort() : false;
}

/**
Starts recording or playback on the stream.
*/
bool Audio::Stream::start()
{
    return d ? d->start() : false;
}

/**
Waits for the streamed buffers to drain and stops playback afterwards.
*/
bool Audio::Stream::stop()
{
    return d ? d->stop() : false;
}

void Audio::Stream::playback(char* pcm, Format fmt, quint32 frames,
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

void Audio::Stream::playback(const QString& fileName)
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

bool Audio::Stream::isOpen() const
{
    return d ? d->audio.isStreamOpen() : false;
}

bool Audio::Stream::isRunning() const
{
    return d ? d->audio.isStreamRunning() : false;
}

void Audio::Stream::onRecorded(RecordFunc event)
{
    if (!d)
        return;

    d->evInput = event;
}

}
