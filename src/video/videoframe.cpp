/*
    Copyright © 2014-2015 by The qTox Project

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

#include <iostream>

#include <QMutexLocker>
#include <QDebug>
#include <vpx/vpx_image.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include "videoframe.h"
#include "camerasource.h"

/**
@class VideoFrame

VideoFrame takes ownership of an AVFrame* and allows fast conversions to other formats
Ownership of all video frame buffers is kept by the VideoFrame, even after conversion
All references to the frame data become invalid when the VideoFrame is deleted
We try to avoid pixel format conversions as much as possible, at the cost of some memory
All methods are thread-safe. If provided freelistCallback will be called by the destructor,
unless releaseFrame was called in between.
*/

VideoFrame::VideoFrame(AVFrame* frame, int w, int h, int fmt, std::function<void()> freelistCallback)
    : freelistCallback{freelistCallback},
      frameOther{nullptr}, frameYUV420{nullptr}, frameRGB24{nullptr},
      width{w}, height{h}, pixFmt{fmt}
{
    // Silences pointless swscale warning spam
    // See libswscale/utils.c:1153 @ 74f0bd3
    frame->color_range = AVCOL_RANGE_MPEG;
    if (pixFmt == AV_PIX_FMT_YUVJ420P)
        pixFmt = AV_PIX_FMT_YUV420P;
    else if (pixFmt == AV_PIX_FMT_YUVJ411P)
        pixFmt = AV_PIX_FMT_YUV411P;
    else if (pixFmt == AV_PIX_FMT_YUVJ422P)
        pixFmt = AV_PIX_FMT_YUV422P;
    else if (pixFmt == AV_PIX_FMT_YUVJ444P)
        pixFmt = AV_PIX_FMT_YUV444P;
    else if (pixFmt == AV_PIX_FMT_YUVJ440P)
        pixFmt = AV_PIX_FMT_YUV440P;
    else
        frame->color_range = AVCOL_RANGE_UNSPECIFIED;

    if (pixFmt == AV_PIX_FMT_YUV420P) {
        frameYUV420 = frame;
    } else if (pixFmt == AV_PIX_FMT_RGB24) {
        frameRGB24 = frame;
    } else {
        frameOther = frame;
    }
}

VideoFrame::VideoFrame(AVFrame* frame, std::function<void()> freelistCallback)
    : VideoFrame{frame, frame->width, frame->height, frame->format, freelistCallback}
{
}

VideoFrame::VideoFrame(AVFrame* frame)
    : VideoFrame{frame, frame->width, frame->height, frame->format, nullptr}
{
}

/**
@brief VideoFrame constructor. Disable copy.
@note Use a shared_ptr if you need copies.
*/
VideoFrame::~VideoFrame()
{
    if (freelistCallback)
        freelistCallback();

    releaseFrameLockless();
}

/**
@brief Converts the VideoFrame to a QImage that shares our internal video buffer.
@param size Size of resulting image.
@return Converted image to RGB24 color model.
*/
QImage VideoFrame::toQImage(QSize size)
{
    if (!convertToRGB24(size))
        return QImage();

    QMutexLocker locker(&biglock);

    return QImage(*frameRGB24->data, frameRGB24->width, frameRGB24->height, *frameRGB24->linesize, QImage::Format_RGB888);
}

/**
@brief Converts the VideoFrame to a vpx_image_t.
Converts the VideoFrame to a vpx_image_t that shares our internal video buffer.
@return Converted image to vpx_image format.
*/
vpx_image *VideoFrame::toVpxImage()
{
    vpx_image* img = vpx_img_alloc(nullptr, VPX_IMG_FMT_I420, width, height, 0);

    if (!convertToYUV420())
        return img;

    for (int i = 0; i < 3; i++)
    {
        int dstStride = img->stride[i];
        int srcStride = frameYUV420->linesize[i];
        int minStride = std::min(dstStride, srcStride);
        int size = (i == 0) ? img->d_h : img->d_h / 2;

        for (int j = 0; j < size; j++)
        {
            uint8_t *dst = img->planes[i] + dstStride * j;
            uint8_t *src = frameYUV420->data[i] + srcStride * j;
            memcpy(dst, src, minStride);
        }
    }
    return img;
}

bool VideoFrame::convertToRGB24(QSize size)
{
    QMutexLocker locker(&biglock);

    AVFrame* sourceFrame;
    if (frameOther)
    {
        sourceFrame = frameOther;
    }
    else if (frameYUV420)
    {
        sourceFrame = frameYUV420;
    }
    else
    {
        qWarning() << "None of the frames are valid! Did someone release us?";
        return false;
    }
    //std::cout << "converting to RGB24" << std::endl;

    if (size.isEmpty())
    {
        size.setWidth(sourceFrame->width);
        size.setHeight(sourceFrame->height);
    }

    if (frameRGB24)
    {
        if (frameRGB24->width == size.width() && frameRGB24->height == size.height())
            return true;

        av_free(frameRGB24->opaque);
        av_frame_unref(frameRGB24);
        av_frame_free(&frameRGB24);
    }

    frameRGB24=av_frame_alloc();
    if (!frameRGB24)
    {
        qCritical() << "av_frame_alloc failed";
        return false;
    }

    int imgBufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, size.width(), size.height(), 1);
    uint8_t* buf = (uint8_t*)av_malloc(imgBufferSize);
    if (!buf)
    {
        qCritical() << "av_malloc failed";
        av_frame_free(&frameRGB24);
        return false;
    }
    frameRGB24->opaque = buf;

    uint8_t** data = frameRGB24->data;
    int* linesize = frameRGB24->linesize;
    av_image_fill_arrays(data, linesize, buf, AV_PIX_FMT_RGB24, size.width(), size.height(), 1);
    frameRGB24->width = size.width();
    frameRGB24->height = size.height();

    // Bilinear is better for shrinking, bicubic better for upscaling
    int resizeAlgo = size.width()<=width ? SWS_BILINEAR : SWS_BICUBIC;

    SwsContext *swsCtx =  sws_getContext(width, height, (AVPixelFormat)pixFmt,
                                          size.width(), size.height(), AV_PIX_FMT_RGB24,
                                          resizeAlgo, nullptr, nullptr, nullptr);
    sws_scale(swsCtx, (uint8_t const * const *)sourceFrame->data,
                sourceFrame->linesize, 0, height,
                frameRGB24->data, frameRGB24->linesize);
    sws_freeContext(swsCtx);

    return true;
}

bool VideoFrame::convertToYUV420()
{
    QMutexLocker locker(&biglock);

    if (frameYUV420)
        return true;

    AVFrame* sourceFrame;
    if (frameOther)
    {
        sourceFrame = frameOther;
    }
    else if (frameRGB24)
    {
        sourceFrame = frameRGB24;
    }
    else
    {
        qCritical() << "None of the frames are valid! Did someone release us?";
        return false;
    }
    //std::cout << "converting to YUV420" << std::endl;

    frameYUV420=av_frame_alloc();
    if (!frameYUV420)
    {
        qCritical() << "av_frame_alloc failed";
        return false;
    }

    int imgBufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    uint8_t* buf = (uint8_t*)av_malloc(imgBufferSize);
    if (!buf)
    {
        qCritical() << "av_malloc failed";
        av_frame_free(&frameYUV420);
        return false;
    }
    frameYUV420->opaque = buf;

    uint8_t** data = frameYUV420->data;
    int* linesize = frameYUV420->linesize;
    av_image_fill_arrays(data, linesize, buf, AV_PIX_FMT_YUV420P, width, height, 1);

    SwsContext *swsCtx =  sws_getContext(width, height, (AVPixelFormat)pixFmt,
                                          width, height, AV_PIX_FMT_YUV420P,
                                          SWS_BILINEAR, nullptr, nullptr, nullptr);
    sws_scale(swsCtx, (uint8_t const * const *)sourceFrame->data,
                sourceFrame->linesize, 0, height,
                frameYUV420->data, frameYUV420->linesize);
    sws_freeContext(swsCtx);

    return true;
}

/**
@brief Frees all frame memory.

Frees all internal buffers and frame data, removes the freelistCallback
This makes all converted objects that shares our internal buffers invalid.
*/
void VideoFrame::releaseFrame()
{
    QMutexLocker locker(&biglock);
    freelistCallback = nullptr;
    releaseFrameLockless();
}

void VideoFrame::releaseFrameLockless()
{
    if (frameOther)
    {
        av_free(frameOther->opaque);
        av_frame_unref(frameOther);
        av_frame_free(&frameOther);
    }
    if (frameYUV420)
    {
        av_free(frameYUV420->opaque);
        av_frame_unref(frameYUV420);
        av_frame_free(&frameYUV420);
    }
    if (frameRGB24)
    {
        av_free(frameRGB24->opaque);
        av_frame_unref(frameRGB24);
        av_frame_free(&frameRGB24);
    }
}

/**
@brief Return the size of the original frame
@return The size of the original frame
*/
QSize VideoFrame::getSize()
{
    return {width, height};
}
