#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QSize>

class VideoSource
{
public:
    virtual void* getData() = 0;
    virtual int getDataSize() = 0;
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual QSize resolution() = 0;
    virtual double fps() = 0;
    virtual void subscribe() = 0;
    virtual void unsubscribe() = 0;
};

#endif // VIDEOSOURCE_H
