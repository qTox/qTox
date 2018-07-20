#ifndef IAUDIOSINK_H
#define IAUDIOSINK_H

#include <cassert>

#include <QObject>

class IAudioSink : public QObject
{
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
