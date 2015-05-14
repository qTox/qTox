/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include <QMutexLocker>
#include <QDebug>
#include <vpx/vpx_image.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}
#include "videoframe.h"

VideoFrame::VideoFrame(AVFrame* frame, int w, int h, int fmt, std::function<void()> freelistCallback)
    : freelistCallback{freelistCallback},
      frameOther{nullptr}, frameYUV420{nullptr}, frameRGB24{nullptr},
      width{w}, height{h}, pixFmt{fmt}
{
    if (pixFmt == AV_PIX_FMT_YUV420P)
        frameYUV420 = frame;
    else if (pixFmt == AV_PIX_FMT_RGB24)
        frameRGB24 = frame;
    else
        frameOther = frame;
}

VideoFrame::VideoFrame(AVFrame* frame, std::function<void()> freelistCallback)
    : VideoFrame{frame, frame->width, frame->height, frame->format, freelistCallback}
{
}

VideoFrame::VideoFrame(AVFrame* frame)
    : VideoFrame{frame, frame->width, frame->height, frame->format, nullptr}
{
}

VideoFrame::~VideoFrame()
{
    if (freelistCallback)
        freelistCallback();

    releaseFrameLockless();
}

QImage VideoFrame::toQImage()
{
    if (!convertToRGB24())
        return QImage();

    QMutexLocker locker(&biglock);

    return QImage(*frameRGB24->data, width, height, *frameRGB24->linesize, QImage::Format_RGB888);
}

vpx_image *VideoFrame::toVpxImage()
{
    // libvpx doesn't provide a clean way to reuse an existing external buffer
    // so we'll manually fill-in the vpx_image fields and hope for the best.
    vpx_image* img = new vpx_image;
    memset(img, 0, sizeof(vpx_image));

    if (!convertToYUV420())
        return img;

    img->w = img->d_w = width;
    img->h = img->d_h = height;
    img->fmt = VPX_IMG_FMT_I420;
    img->planes[0] = frameYUV420->data[0];
    img->planes[1] = frameYUV420->data[1];
    img->planes[2] = frameYUV420->data[2];
    img->planes[3] = nullptr;
    img->stride[0] = frameYUV420->linesize[0];
    img->stride[1] = frameYUV420->linesize[1];
    img->stride[2] = frameYUV420->linesize[2];
    img->stride[3] = frameYUV420->linesize[3];
    return img;
}

bool VideoFrame::convertToRGB24()
{
    QMutexLocker locker(&biglock);

    if (frameRGB24)
        return true;

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
        qCritical() << "None of the frames are valid! Did someone release us?";
        return false;
    }

    frameRGB24=av_frame_alloc();
    if (!frameRGB24)
    {
        qCritical() << "av_frame_alloc failed";
        return false;
    }

    uint8_t* buf = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24, width, height));
    if (!buf)
    {
        qCritical() << "av_malloc failed";
        av_frame_free(&frameRGB24);
        return false;
    }
    frameRGB24->opaque = buf;

    avpicture_fill((AVPicture*)frameRGB24, buf, AV_PIX_FMT_RGB24, width, height);

    SwsContext *swsCtx =  sws_getContext(width, height, (AVPixelFormat)pixFmt,
                                          width, height, AV_PIX_FMT_RGB24,
                                          SWS_BILINEAR, nullptr, nullptr, nullptr);
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

    frameYUV420=av_frame_alloc();
    if (!frameYUV420)
    {
        qCritical() << "av_frame_alloc failed";
        return false;
    }

    uint8_t* buf = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24, width, height));
    if (!buf)
    {
        qCritical() << "av_malloc failed";
        av_frame_free(&frameYUV420);
        return false;
    }
    frameYUV420->opaque = buf;

    avpicture_fill((AVPicture*)frameYUV420, buf, AV_PIX_FMT_YUV420P, width, height);

    SwsContext *swsCtx =  sws_getContext(width, height, (AVPixelFormat)pixFmt,
                                          width, height, AV_PIX_FMT_YUV420P,
                                          SWS_BILINEAR, nullptr, nullptr, nullptr);
    sws_scale(swsCtx, (uint8_t const * const *)sourceFrame->data,
                sourceFrame->linesize, 0, height,
                frameYUV420->data, frameYUV420->linesize);
    sws_freeContext(swsCtx);

    return true;
}

void VideoFrame::releaseFrame()
{
    QMutexLocker locker(&biglock);
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
