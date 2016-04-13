/*
    Copyright © 2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}
#include "corevideosource.h"
#include "videoframe.h"

CoreVideoSource::CoreVideoSource()
    : subscribers{0}, deleteOnClose{false},
    stopped{false}
{
}

void CoreVideoSource::pushFrame(const vpx_image_t* vpxframe)
{
    if (stopped)
        return;

    QMutexLocker locker(&biglock);

    std::shared_ptr<VideoFrame> vframe;
    AVFrame* avframe;
    uint8_t* buf;
    int width = vpxframe->d_w, height = vpxframe->d_h;
    int dstStride, srcStride, minStride;

    if (subscribers <= 0)
        return;

    avframe = av_frame_alloc();
    if (!avframe)
        return;
    avframe->width = width;
    avframe->height = height;
    avframe->format = AV_PIX_FMT_YUV420P;

    int imgBufferSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
    buf = (uint8_t*)av_malloc(imgBufferSize);
    if (!buf)
    {
        av_frame_free(&avframe);
        return;
    }
    avframe->opaque = buf;

    uint8_t** data = avframe->data;
    int* linesize = avframe->linesize;
    av_image_fill_arrays(data, linesize, buf, AV_PIX_FMT_YUV420P, width, height, 1);

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
}

bool CoreVideoSource::subscribe()
{
    QMutexLocker locker(&biglock);
    ++subscribers;
    return true;
}

void CoreVideoSource::unsubscribe()
{
    biglock.lock();
    if (--subscribers == 0)
    {
        if (deleteOnClose)
        {
            biglock.unlock();
            // DANGEROUS: No member access after this point, that's why we manually unlock
            delete this;
            return;
        }
    }
    biglock.unlock();
}

void CoreVideoSource::setDeleteOnClose(bool newstate)
{
    QMutexLocker locker(&biglock);
    deleteOnClose = newstate;
}

void CoreVideoSource::stopSource()
{
    QMutexLocker locker(&biglock);
    stopped = true;
    emit sourceStopped();
}

void CoreVideoSource::restartSource()
{
    QMutexLocker locker(&biglock);
    stopped = false;
}
