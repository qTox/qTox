#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QObject>
#include <QSize>
#include <QRgb>

struct VideoFrame
{
    enum ColorFormat
    {
        BGR,
        YUV,
    };

    QByteArray data;
    QSize resolution;
    ColorFormat format;

    void setNull()
    {
        data = QByteArray();
    }

    bool isNull()
    {
        return data.isEmpty();
    }

    // assumes format is BGR
    QRgb getPixel(int x, int y)
    {
        char b = data.data()[resolution.width() * 3 * y + x * 3 + 0];
        char g = data.data()[resolution.width() * 3 * y + x * 3 + 1];
        char r = data.data()[resolution.width() * 3 * y + x * 3 + 2];

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
    virtual VideoFrame::ColorFormat getColorFormat() = 0;

signals:
    void frameAvailable(const VideoFrame frame);

};

#endif // VIDEOSOURCE_H
