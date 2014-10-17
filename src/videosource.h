#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QObject>
#include <QSize>
#include <QRgb>

struct VideoFrame
{
    enum ColorFormat
    {
        NONE,
        BGR,
        YUV,
    };

    QByteArray frameData;
    QSize resolution;
    ColorFormat format;

    VideoFrame() : format(NONE) {}
    VideoFrame(QByteArray d, QSize r, ColorFormat f) : frameData(d), resolution(r), format(f) {}

    void setNull()
    {
        frameData = QByteArray();
    }

    bool isNull()
    {
        return frameData.isEmpty();
    }

    // assumes format is BGR
    QRgb getPixel(int x, int y)
    {
        char b = frameData.data()[resolution.width() * 3 * y + x * 3 + 0];
        char g = frameData.data()[resolution.width() * 3 * y + x * 3 + 1];
        char r = frameData.data()[resolution.width() * 3 * y + x * 3 + 2];

        return qRgb(r, g, b);
    }
};

Q_DECLARE_METATYPE(VideoFrame)

class VideoSource : public QObject
{
    Q_OBJECT

public:
    virtual void subscribe() = 0;
    virtual void unsubscribe() = 0;

signals:
    void frameAvailable(const VideoFrame frame);

};

#endif // VIDEOSOURCE_H
