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

#include <QObject>
#include <memory>

class IAudioSink;
class IAudioSource;
class Audio : public QObject
{
    Q_OBJECT

public:
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

    virtual std::unique_ptr<IAudioSink> makeSink() = 0;
    virtual std::unique_ptr<IAudioSource> makeSource() = 0;

protected:
    // Public default audio settings
    // Samplerate for Tox calls and sounds
    static constexpr uint32_t AUDIO_SAMPLE_RATE = 48000;
    static constexpr uint32_t AUDIO_FRAME_DURATION = 20;
    static constexpr uint32_t AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL =
        AUDIO_FRAME_DURATION * AUDIO_SAMPLE_RATE / 1000;
    uint32_t AUDIO_FRAME_SAMPLE_COUNT_TOTAL = 0;
};

#endif // AUDIO_H
