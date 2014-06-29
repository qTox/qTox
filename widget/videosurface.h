#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include "vpx/vpx_image.h"

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    VideoSurface();
    bool start(const QVideoSurfaceFormat &format);
    bool present(const QVideoFrame &frame);
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;

signals:
    // Slots MUST be called with a direct or blocking connection, or img may die before they return !
    void videoFrameReady(vpx_image img);

private:
    QVideoSurfaceFormat mVideoFormat;
    vpx_image_t input;
};

#endif // VIDEOSURFACE_H
