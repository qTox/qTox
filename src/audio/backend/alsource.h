#ifndef ALSOURCE_H
#define ALSOURCE_H

#include "src/audio/iaudiosource.h"
#include <QMutex>
#include <QObject>

class OpenAL;
class AlSource : public IAudioSource
{
    Q_OBJECT
public:
    AlSource(OpenAL& al);
    AlSource(AlSource& src) = delete;
    AlSource& operator=(const AlSource&) = delete;
    AlSource(AlSource&& other) = delete;
    AlSource& operator=(AlSource&& other) = delete;
    ~AlSource();

    operator bool() const;

    void kill();

private:
    OpenAL& audio;
    bool killed = false;
    mutable QMutex killLock;
};

#endif // ALSOURCE_H
