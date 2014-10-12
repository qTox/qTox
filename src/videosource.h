#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QObject>
#include <QSize>

class VideoSource : public QObject
{
    Q_OBJECT
public:
    virtual void* getData() = 0; // a pointer to a frame
    virtual int getDataSize() = 0; // size of a frame in bytes

    virtual void lock() = 0; // locks a frame so that it can't change
    virtual void unlock() = 0;

    virtual QSize resolution() = 0; // resolution of a frame

    virtual void subscribe() = 0;
    virtual void unsubscribe() = 0;

signals:
    void frameAvailable();

};

#endif // VIDEOSOURCE_H
