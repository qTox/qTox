#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <QObject>

class Audio;
class AudioSource : public QObject
{
    Q_OBJECT
public:
    AudioSource(Audio& audio);
    AudioSource(AudioSource &src) = delete;
    AudioSource & operator=(const AudioSource&) = delete;
    AudioSource(AudioSource&& other);
    AudioSource & operator=(AudioSource&& other);
    ~AudioSource();

    operator bool() const;

signals:
    void frameAvailable(const int16_t* pcm, size_t sample_count, uint8_t channels,
                        uint32_t sampling_rate);

private:
    Audio* audio;
};

#endif // AUDIOSOURCE_H
