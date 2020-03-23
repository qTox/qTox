/*
    Copyright © 2015-2019 The qTox Project Contributors

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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#pragma GCC diagnostic pop
}

#include "corevideosource.h"
#include "videoframe.h"

/**
 * @class CoreVideoSource
 * @brief A VideoSource that emits frames received by Core.
 */

/**
 * @var std::atomic_int subscribers
 * @brief Number of suscribers
 *
 * @var std::atomic_bool deleteOnClose
 * @brief If true, self-delete after the last suscriber is gone
 */

/**
 * @brief CoreVideoSource constructor.
 * @note Only CoreAV should create a CoreVideoSource since
 * only CoreAV can push images to it.
 */
CoreVideoSource::CoreVideoSource()
    : subscribers{0}
    , deleteOnClose{false}
    , stopped{false}
{
}

/**
 * @brief Makes a copy of the vpx_image_t and emits it as a new VideoFrame.
 * @param vpxframe Frame to copy.
 */
void CoreVideoSource::pushFrame(const vpx_image_t* vpxframe)
{
    if (stopped)
        return;

    QMutexLocker locker(&biglock);

    std::shared_ptr<VideoFrame> vframe;
    int width = vpxframe->d_w;
    int height = vpxframe->d_h;

    if (subscribers <= 0)
        return;

    AVFrame* avframe = av_frame_alloc();
    if (!avframe)
        return;

    avframe->width = width;
    avframe->height = height;
    avframe->format = AV_PIX_FMT_YUV420P;

    int bufSize =
        av_image_alloc(avframe->data, avframe->linesize, width, height,
                       static_cast<AVPixelFormat>(AV_PIX_FMT_YUV420P), VideoFrame::dataAlignment);

    if (bufSize < 0) {
        av_frame_free(&avframe);
        return;
    }

    for (int i = 0; i < 3; ++i) {
        int dstStride = avframe->linesize[i];
        int srcStride = vpxframe->stride[i];
        int minStride = std::min(dstStride, srcStride);
        int size = (i == 0) ? height : height / 2;

        for (int j = 0; j < size; ++j) {
            uint8_t* dst = avframe->data[i] + dstStride * j;
            uint8_t* src = vpxframe->planes[i] + srcStride * j;
            memcpy(dst, src, minStride);
        }
    }

    vframe = std::make_shared<VideoFrame>(id, avframe, true);
    emit frameAvailable(vframe);
}

void CoreVideoSource::subscribe()
{
    QMutexLocker locker(&biglock);
    ++subscribers;
}

void CoreVideoSource::unsubscribe()
{
    biglock.lock();
    if (--subscribers == 0) {
        if (deleteOnClose) {
            biglock.unlock();
            // DANGEROUS: No member access after this point, that's why we manually unlock
            delete this;
            return;
        }
    }
    biglock.unlock();
}

/**
 * @brief Setup delete on close
 * @param If true, self-delete after the last suscriber is gone
 */
void CoreVideoSource::setDeleteOnClose(bool newstate)
{
    QMutexLocker locker(&biglock);
    deleteOnClose = newstate;
}

/**
 * @brief Stopping the source.
 * @see The callers in CoreAV for the rationale
 *
 * Stopping the source will block any pushFrame calls from doing anything
 */
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
