#include "videosurface.h"
#include "core.h"
#include <QVideoFrame>
#include <QDebug>

VideoSurface::VideoSurface()
    : QAbstractVideoSurface()
{
    vpx_img_alloc(&input, VPX_IMG_FMT_YV12, TOXAV_VIDEO_WIDTH, TOXAV_VIDEO_HEIGHT, 1);
}

bool VideoSurface::start(const QVideoSurfaceFormat &format)
{
    mVideoFormat = format;
    //start only if format is UYVY, dont handle other format now
    if( format.pixelFormat() == QVideoFrame::Format_YV12 ){
        QAbstractVideoSurface::start(format);
        return true;
    } else {
        return false;
    }
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    /*
    mFrame = frame;

    qDebug() << QString("Video: Frame format is %1").arg(mFrame.pixelFormat());

    stop();

    //this is necessary to get valid data from frame
    mFrame.map(QAbstractVideoBuffer::ReadOnly);

    uchar* data = new uchar[frame.mappedBytes()];
    memcpy(data, frame.bits(), frame.mappedBytes());

    input.planes[VPX_PLANE_Y] = data;
    input.planes[VPX_PLANE_U] = data + (frame.bytesPerLine() * frame.height());
    input.planes[VPX_PLANE_V] = input.planes[VPX_PLANE_U] + (frame.bytesPerLine()/2 * frame.height()/2);
    input.planes[VPX_PLANE_ALPHA] = nullptr;

    //qDebug() << QString("Got %1 bytes, first plane is %2 bytes long")
    //            .arg(frame.mappedBytes()).arg(frame.bytesPerLine() * frame.height());

    // Slots MUST be called with a direct or blocking connection, or input may die before they return !
    emit videoFrameReady(input);


    QImage lastImage( mFrame.size(), QImage::Format_RGB16);
    const uchar *src = mFrame.bits();
    uchar *dst = lastImage.bits();
    const int srcLineStep = mFrame.bytesPerLine();
    const int dstLineStep = lastImage.bytesPerLine();
    const int h = mFrame.height();
    const int w = mFrame.width();

    for (int y=0; y < h; y++) {
        //this function you can find in qgraphicsvideoitem_maemo5.cpp,
        //link is mentioned above
        uyvy422_to_rgb16_line_neon(dst, src, w);
        src += srcLineStep;
        dst += dstLineStep;
    }

    mLastFrame = QPixmap::fromImage(lastImage);
    //emit signal, other can handle it and do necessary processing
    emit frameUpdated(mLastFrame);

    delete[] data;
    mFrame.unmap();

*/
    return true;
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle) {
        qDebug() << "Video: No handle";
        return QList<QVideoFrame::PixelFormat>() <<  QVideoFrame::Format_YV12;
    } else {
        qDebug() << "Video: Handle type is not NoHandle";
        return QList<QVideoFrame::PixelFormat>();
    }
}
