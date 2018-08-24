#ifndef IAUDIOSINK_H
#define IAUDIOSINK_H

#include <cassert>

#include <QObject>

/**
 * @brief The IAudioSink class represents an interface to devices that can play audio.
 *
 * @enum IAudioSink::Sound
 * @brief Provides the different sounds for use in the getSound function.
 * @see getSound
 *
 * @value NewMessage Returns the new message notification sound.
 * @value Test Returns the test sound.
 * @value IncomingCall Returns the incoming call sound.
 * @value OutgoingCall Returns the outgoing call sound.
 *
 * @fn QString IAudioSink::getSound(Sound s)
 * @brief Function to get the path of the requested sound.
 *
 * @param s Name of the sound to get the path of.
 * @return The path of the requested sound.
 *
 * @fn void IAudioSink::playAudioBuffer(const int16_t* data, int samples,
 *                                  unsigned channels, int sampleRate)
 * @brief adds a number of audio frames to the play buffer
 *
 * @param[in] data 16bit mono or stereo PCM data with alternating channel
 *            mapping for stereo (LRLR)
 * @param[in] samples number of samples per channel
 * @param[in] channels number of channels, currently 1 or 2 is supported
 * @param[in] sampleRate sample rate in Hertz
 *
 * @fn void IAudioSink::playMono16Sound(const Sound& sound)
 * @brief Play a 44100Hz mono 16bit PCM sound from the builtin sounds.
 *
 * @param[in] sound The sound to play
 *
 * @fn void IAudioSink::startLoop()
 * @brief starts looping the sound played with playMono16Sound(...)
 *
 * @fn void IAudioSink::stopLoop()
 * @brief stops looping the sound played with playMono16Sound(...)
 *
 */

class IAudioSink : public QObject
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
            return QStringLiteral(":/audio/notification.pcm");
        case Sound::NewMessage:
            return QStringLiteral(":/audio/notification.pcm");
        case Sound::IncomingCall:
            return QStringLiteral(":/audio/ToxIncomingCall.pcm");
        case Sound::OutgoingCall:
            return QStringLiteral(":/audio/ToxOutgoingCall.pcm");
        case Sound::CallEnd:
            return QStringLiteral(":/audio/ToxEndCall.pcm");
        }
        assert(false);
        return {};
    }

    virtual ~IAudioSink() {}
    virtual void playAudioBuffer(const int16_t* data, int samples, unsigned channels,
                         int sampleRate) const = 0;
    virtual void playMono16Sound(const Sound& sound) = 0;
    virtual void startLoop() = 0;
    virtual void stopLoop() = 0;

    virtual operator bool() const = 0;

signals:
    void finishedPlaying();
    void invalidated();
};

#endif // IAUDIOSINK_H