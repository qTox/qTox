extern "C" {
#include <libavcodec/avcodec.h>
}
#include "corevideosource.h"
#include "videoframe.h"

CoreVideoSource::CoreVideoSource()
    : subscribers{0}, deleteOnClose{false},
      biglock{false}
{
}

void CoreVideoSource::pushFrame(const vpx_image_t* vpxframe)
{
    // Fast lock
    {
        bool expected = false;
        while (!biglock.compare_exchange_weak(expected, true))
            expected = false;
    }

    std::shared_ptr<VideoFrame> vframe;
    AVFrame* avframe;
    uint8_t* buf;
    int width = vpxframe->d_w, height = vpxframe->d_h;
    int dstStride, srcStride, minStride;

    if (subscribers <= 0)
        goto end;

    avframe = av_frame_alloc();
    if (!avframe)
        goto end;
    avframe->width = width;
    avframe->height = height;
    avframe->format = AV_PIX_FMT_YUV420P;

    buf = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, width, height));
    if (!buf)
    {
        av_frame_free(&avframe);
        goto end;
    }
    avframe->opaque = buf;

    avpicture_fill((AVPicture*)avframe, buf, AV_PIX_FMT_YUV420P, width, height);

    dstStride=avframe->linesize[0], srcStride=vpxframe->stride[0], minStride=std::min(dstStride, srcStride);
    for (int i=0; i<height; i++)
        memcpy(avframe->data[0]+dstStride*i, vpxframe->planes[0]+srcStride*i, minStride);
    dstStride=avframe->linesize[1], srcStride=vpxframe->stride[1], minStride=std::min(dstStride, srcStride);
    for (int i=0; i<height/2; i++)
        memcpy(avframe->data[1]+dstStride*i, vpxframe->planes[1]+srcStride*i, minStride);
    dstStride=avframe->linesize[2], srcStride=vpxframe->stride[2], minStride=std::min(dstStride, srcStride);
    for (int i=0; i<height/2; i++)
        memcpy(avframe->data[2]+dstStride*i, vpxframe->planes[2]+srcStride*i, minStride);

    vframe = std::make_shared<VideoFrame>(avframe);
    emit frameAvailable(vframe);

end:
    biglock = false;
}

bool CoreVideoSource::subscribe()
{
    // Fast lock
    {
        bool expected = false;
        while (!biglock.compare_exchange_weak(expected, true))
            expected = false;
    }
    ++subscribers;
    biglock = false;
    return true;
}

void CoreVideoSource::unsubscribe()
{
    // Fast lock
    {
        bool expected = false;
        while (!biglock.compare_exchange_weak(expected, true))
            expected = false;
    }
    if (--subscribers == 0)
    {
        if (deleteOnClose)
        {
            biglock = false;
            delete this;
            return;
        }
    }
    biglock = false;
}

void CoreVideoSource::setDeleteOnClose(bool newstate)
{
    // Fast lock
    {
        bool expected = false;
        while (!biglock.compare_exchange_weak(expected, true))
            expected = false;
    }
    deleteOnClose = newstate;
    biglock = false;
}
