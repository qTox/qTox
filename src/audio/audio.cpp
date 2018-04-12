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

#include "audio.h"
#include "src/audio/backend/openal.h"
#ifdef USE_FILTERAUDIO
#include "src/audio/backend/openal2.h"
#endif
#include "src/persistence/settings.h"

#include <QDebug>

#include <cassert>

/**
 * @class Audio
 *
 * @enum Audio::Sound
 * @brief Provides the different sounds for use in the getSound function.
 * @see getSound
 *
 * @value NewMessage Returns the new message notification sound.
 * @value Test Returns the test sound.
 * @value IncomingCall Returns the incoming call sound.
 * @value OutgoingCall Returns the outgoing call sound.
 *
 * @fn QString Audio::getSound(Sound s)
 * @brief Function to get the path of the requested sound.
 *
 * @param s Name of the sound to get the path of.
 * @return The path of the requested sound.
 *
 * @fn void Audio::frameAvailable(const int16_t *pcm, size_t sample_count, uint8_t channels,
 * uint32_t sampling_rate);
 *
 * When there are input subscribers, we regularly emit captured audio frames with this signal
 * Always connect with a blocking queued connection lambda, else the behaviour is undefined
 *
 * @var Audio::AUDIO_SAMPLE_RATE
 * @brief The next best Opus would take is 24k
 *
 * @var Audio::AUDIO_FRAME_DURATION
 * @brief In milliseconds
 *
 * @var Audio::AUDIO_FRAME_SAMPLE_COUNT
 * @brief Frame sample count
 *
 * @fn qreal Audio::outputVolume() const
 * @brief Returns the current output volume (between 0 and 1)
 *
 * @fn void Audio::setOutputVolume(qreal volume)
 * @brief Set the master output volume.
 *
 * @param[in] volume   the master volume (between 0 and 1)
 *
 * @fn qreal Audio::minInputGain() const
 * @brief The minimum gain value for an input device.
 *
 * @return minimum gain value in dB
 *
 * @fn void Audio::setMinInputGain(qreal dB)
 * @brief Set the minimum allowed gain value in dB.
 *
 * @note Default is -30dB; usually you don't need to alter this value;
 *
 * @fn qreal Audio::maxInputGain() const
 * @brief The maximum gain value for an input device.
 *
 * @return maximum gain value in dB
 *
 * @fn void Audio::setMaxInputGain(qreal dB)
 * @brief Set the maximum allowed gain value in dB.
 *
 * @note Default is 30dB; usually you don't need to alter this value.
 *
 * @fn void Audio::subscribeInput()
 * @brief Subscribe to capture sound from the opened input device.
 *
 * If the input device is not open, it will be opened before capturing.
 *
 * @fn void Audio::unsubscribeInput()
 * @brief Unsubscribe from capturing from an opened input device.
 *
 * If the input device has no more subscriptions, it will be closed.
 *
 * @fn void Audio::playMono16Sound(const QString& path)
 * @brief Play a 44100Hz mono 16bit PCM sound from a file
 *
 * @param[in] path the path to the sound file
 *
 * @fn void Audio::playMono16Sound(const QByteArray& data)
 * @brief Play a 44100Hz mono 16bit PCM sound
 *
 * @param[in] data 44100Hz mono 16bit PCM data in host byte order
 *
 * @fn void Audio::playAudioBuffer(uint sourceId, const int16_t* data, int samples,
 *                                  unsigned channels, int sampleRate)
 * @brief adds a number of audio frames to the play buffer
 *
 * @param[in] sourceId id obtained by subscribeOutput(uint &)
 * @param[in] data 16bit mono or stereo PCM data with alternating channel
 *            mapping for stereo (LRLR)
 * @param[in] samples number of samples per channel
 * @param[in] channels number of channels, currently 1 or 2 is supported
 * @param[in] sampleRate sample rate in Hertz
 *
 * @fn bool Audio::isOutputReady() const
 * @brief check if the output is ready to play audio
 *
 * @return true if the output device is open, false otherwise
 *
 * @fn QStringList Audio::outDeviceNames()
 * @brief Get the names of available output devices
 *
 * @return list of output devices
 *
 * @fn QStringList Audio::inDeviceNames()
 * @brief Get the names of available input devices
 *
 * @return list of input devices
 *
 * @fn void Audio::subscribeOutput(uint& sid)
 * @brief register a new output source
 *
 * param[out] sid contains the sourceId if source creation was successful,
 *                unchanged otherwise
 *
 * @fn void Audio::unsubscribeOutput(uint& sid)
 * @brief unregisters an output source
 *
 * param[out] sid contains 0 if source deletion was successful,
 *                unchanged otherwise
 *
 * @fn void Audio::startLoop()
 * @brief starts looping the sound played with playMono16Sound()
 *
 * @fn void Audio::stopLoop()
 * @brief stops looping the sound played with playMono16Sound()
 *
 * @fn qreal Audio::inputGain() const
 * @brief get the current input gain
 *
 * @return current input gain in dB
 *
 * @fn void Audio::setInputGain(qreal dB)
 * @brief set the input gain
 *
 * @fn void Audio::getInputThreshold()
 * @brief get the current input threshold
 *
 * @return current input threshold percentage
 *
 * @fn void Audio::setInputThreshold(qreal percent)
 * @brief set the input threshold
 *
 * @param[in] percent the new input threshold percentage
 */

/**
 * @brief Returns the singleton instance.
 */
Audio& Audio::getInstance()
{
    // TODO: replace backend selection by inversion of control
#ifdef USE_FILTERAUDIO
    static bool initialized = false;
    static bool Backend2 = false;

    if (!initialized) {
        Backend2 = Settings::getInstance().getEnableBackend2();
        initialized = true;
    }

    if (Backend2) {
        static OpenAL2 instance;
        return instance;
    } else
#endif
    {
        static OpenAL instance;
        return instance;
    }
}
