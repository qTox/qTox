/*
    Copyright © 2019 by The qTox Project Contributors

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

#pragma once

#include <QObject>
#include <QStringList>
#include <memory>

/**
 * @class IAudioControl
 *
 * @var IAudioControl::AUDIO_SAMPLE_RATE
 * @brief The next best Opus would take is 24k
 *
 * @var IAudioControl::AUDIO_FRAME_DURATION
 * @brief In milliseconds
 *
 * @var IAudioControl::AUDIO_FRAME_SAMPLE_COUNT
 * @brief Frame sample count
 *
 * @fn void IAudioControl::setOutputVolume(qreal volume)
 * @brief Set the master output volume.
 *
 * @param[in] volume   the master volume (between 0 and 1)
 *
 * @fn qreal IAudioControl::minInputGain() const
 * @brief The minimum gain value for an input device.
 *
 * @return minimum gain value in dB
 *
 * @fn void IAudioControl::setMinInputGain(qreal dB)
 * @brief Set the minimum allowed gain value in dB.
 *
 * @note Default is -30dB; usually you don't need to alter this value;
 *
 * @fn qreal IAudioControl::maxInputGain() const
 * @brief The maximum gain value for an input device.
 *
 * @return maximum gain value in dB
 *
 * @fn void IAudioControl::setMaxInputGain(qreal dB)
 * @brief Set the maximum allowed gain value in dB.
 *
 * @note Default is 30dB; usually you don't need to alter this value.
 *
 * @fn bool IAudioControl::isOutputReady() const
 * @brief check if the output is ready to play audio
 *
 * @return true if the output device is open, false otherwise
 *
 * @fn QStringList IAudioControl::outDeviceNames()
 * @brief Get the names of available output devices
 *
 * @return list of output devices
 *
 * @fn QStringList IAudioControl::inDeviceNames()
 * @brief Get the names of available input devices
 *
 * @return list of input devices
 *
 * @fn qreal IAudioControl::inputGain() const
 * @brief get the current input gain
 *
 * @return current input gain in dB
 *
 * @fn void IAudioControl::setInputGain(qreal dB)
 * @brief set the input gain
 *
 * @fn void IAudioControl::getInputThreshold()
 * @brief get the current input threshold
 *
 * @return current input threshold percentage
 *
 * @fn void IAudioControl::setInputThreshold(qreal percent)
 * @brief set the input threshold
 *
 * @param[in] percent the new input threshold percentage
 */

class IAudioSink;
class IAudioSource;
class IAudioControl : public QObject
{
    Q_OBJECT

public:
    virtual ~IAudioControl() = default;
    virtual void setOutputVolume(qreal volume) = 0;
    virtual qreal maxOutputVolume() const = 0;
    virtual qreal minOutputVolume() const = 0;

    virtual qreal minInputGain() const = 0;
    virtual qreal maxInputGain() const = 0;

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
