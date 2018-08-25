/*
    Copyright © 2014-2018 by The qTox Project Contributors

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


#ifndef AUDIO_H
#define AUDIO_H

#include <atomic>
#include <cmath>

#include <QMutex>
#include <QObject>
#include <QTimer>

#include <cassert>

class Audio : public QObject
{
    Q_OBJECT

public:
    enum class Sound
    {
        NewMessage,
        Test,
        IncomingCall,
        OutgoingCall,
        CallEnd
    };

    inline static QString getSound(Sound s)
    {
        switch (s) {
        case Sound::Test:
            return QStringLiteral(":/audio/notification.s16le.pcm");
        case Sound::NewMessage:
            return QStringLiteral(":/audio/notification.s16le.pcm");
        case Sound::IncomingCall:
            return QStringLiteral(":/audio/ToxIncomingCall.s16le.pcm");
        case Sound::OutgoingCall:
            return QStringLiteral(":/audio/ToxOutgoingCall.s16le.pcm");
        case Sound::CallEnd:
            return QStringLiteral(":/audio/ToxEndCall.s16le.pcm");
        }
        assert(false);
        return QString();
    }
    static Audio& getInstance();

    virtual qreal outputVolume() const = 0;
    virtual void setOutputVolume(qreal volume) = 0;
    virtual qreal maxOutputVolume() const = 0;
    virtual qreal minOutputVolume() const = 0;

    virtual qreal minInputGain() const = 0;
    virtual void setMinInputGain(qreal dB) = 0;

    virtual qreal maxInputGain() const = 0;
    virtual void setMaxInputGain(qreal dB) = 0;

    virtual qreal inputGain() const = 0;
    virtual void setInputGain(qreal dB) = 0;

    virtual qreal minInputThreshold() const = 0;
    virtual qreal maxInputThreshold() const = 0;

    virtual qreal getInputThreshold() const = 0;
    virtual void setInputThreshold(qreal percent) = 0;

    virtual void reinitInput(const QString& inDevDesc) = 0;
    virtual bool reinitOutput(const QString& outDevDesc) = 0;

    virtual bool isOutputReady() const = 0;

    virtual QStringList outDeviceNames() = 0;
    virtual QStringList inDeviceNames() = 0;

    virtual void subscribeOutput(uint& sourceId) = 0;
    virtual void unsubscribeOutput(uint& sourceId) = 0;

    virtual void subscribeInput() = 0;
    virtual void unsubscribeInput() = 0;

    virtual void startLoop() = 0;
    virtual void stopLoop() = 0;
    virtual void playMono16Sound(const QByteArray& data) = 0;
    virtual void playMono16Sound(const QString& path) = 0;

    virtual void stopActive() = 0;

    virtual void playAudioBuffer(uint sourceId, const int16_t* data, int samples, unsigned channels,
                                 int sampleRate) = 0;

protected:
    // Public default audio settings
    // Samplerate for Tox calls and sounds
    static constexpr uint32_t AUDIO_SAMPLE_RATE = 48000;
    static constexpr uint32_t AUDIO_FRAME_DURATION = 20;
    static constexpr uint32_t AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL =
        AUDIO_FRAME_DURATION * AUDIO_SAMPLE_RATE / 1000;
    uint32_t AUDIO_FRAME_SAMPLE_COUNT_TOTAL = 0;

signals:
    void frameAvailable(const int16_t* pcm, size_t sample_count, uint8_t channels,
                        uint32_t sampling_rate);
    void volumeAvailable(float value);
    void startActive(qreal msec);
};

#endif // AUDIO_H
