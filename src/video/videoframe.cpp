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

#include "videoframe.h"

extern "C"{
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoFrame::VideoFrame(AVFrame* sourceFrame, QRect dimensions, int pixFmt, std::function<void ()> destructCallback, bool freeSourceFrame)
    : sourceDimensions(dimensions),
      sourcePixelFormat(pixFmt),
      freeSourceFrame(freeSourceFrame),
      destructCallback(destructCallback)
{
    frameBuffer[pixFmt][dimensionsToKey(dimensions.size())] = sourceFrame;
}

VideoFrame::VideoFrame(AVFrame* sourceFrame, std::function<void ()> destructCallback, bool freeSourceFrame)
    : VideoFrame(sourceFrame, QRect {0, 0, sourceFrame->width, sourceFrame->height}, sourceFrame->format, destructCallback, freeSourceFrame){}

VideoFrame::VideoFrame(AVFrame* sourceFrame, bool freeSourceFrame)
    : VideoFrame(sourceFrame, QRect {0, 0, sourceFrame->width, sourceFrame->height}, sourceFrame->format, nullptr, freeSourceFrame){}

VideoFrame::~VideoFrame()
{
    frameLock.lockForWrite();

    if(destructCallback && frameBuffer.size() > 0)
    {
        destructCallback();
    }

    deleteFrameBuffer();

    frameLock.unlock();
}

bool VideoFrame::isValid()
{
    frameLock.lockForRead();
    bool retValue = frameBuffer.size() > 0;
    frameLock.unlock();

    return retValue;
}

const AVFrame* VideoFrame::getAVFrame(const QSize& dimensions, const int pixelFormat)
{
    frameLock.lockForRead();

    // We return nullptr if the VideoFrame is no longer valid
    if(frameBuffer.size() == 0)
    {
        frameLock.unlock();
        return nullptr;
    }

    AVFrame* ret = retrieveAVFrame(dimensions, pixelFormat);

    if(ret)
    {
        frameLock.unlock();
        return ret;
    }

    // VideoFrame does not contain an AVFrame to spec, generate one here
    ret = generateAVFrame(dimensions, pixelFormat);

    /*
     * We need to "upgrade" the lock to a write lock so we can update our frameBuffer map.
     *
     * It doesn't matter if another thread obtains the write lock before we finish since it is
     * likely writing to somewhere else. Absolute worst-case scenario, we merely perform the
     * generation process twice, and discard the old result.
     */
    frameLock.unlock();
    frameLock.lockForWrite();

    storeAVFrame(ret, dimensions, pixelFormat);

    frameLock.unlock();
    return ret;
}

void VideoFrame::releaseFrame()
{
    frameLock.lockForWrite();

    deleteFrameBuffer();

    frameLock.unlock();
}

QImage VideoFrame::toQImage(QSize frameSize)
{
    frameLock.lockForRead();

    // We return an empty QImage if the VideoFrame is no longer valid
    if(frameBuffer.size() == 0)
    {
        frameLock.unlock();
        return QImage {};
    }

    if(frameSize.width() == 0 && frameSize.height() == 0)
    {
        frameSize = sourceDimensions.size();
    }

    AVFrame* frame = retrieveAVFrame(frameSize, static_cast<int>(AV_PIX_FMT_RGB24));

    if(frame)
    {
        QImage ret {*(frame->data), frameSize.width(), frameSize.height(), *(frame->linesize), QImage::Format_RGB888};

        frameLock.unlock();
        return ret;
    }

    // VideoFrame does not contain an AVFrame to spec, generate one here
    frame = generateAVFrame(frameSize, static_cast<int>(AV_PIX_FMT_RGB24));

    /*
     * We need to "upgrade" the lock to a write lock so we can update our frameBuffer map.
     *
     * It doesn't matter if another thread obtains the write lock before we finish since it is
     * likely writing to somewhere else. Absolute worst-case scenario, we merely perform the
     * generation process twice, and discard the old result.
     */
    frameLock.unlock();
    frameLock.lockForWrite();

    storeAVFrame(frame, frameSize, static_cast<int>(AV_PIX_FMT_RGB24));

    QImage ret {*(frame->data), frameSize.width(), frameSize.height(), *(frame->linesize), QImage::Format_RGB888};

    frameLock.unlock();
    return ret;
}

vpx_image* VideoFrame::toVpxImage(QSize frameSize)
{
    frameLock.lockForRead();

    // We return nullptr if the VideoFrame is no longer valid
    if(frameBuffer.size() == 0)
    {
        frameLock.unlock();
        return nullptr;
    }

    if(frameSize.width() == 0 && frameSize.height() == 0)
    {
        frameSize = sourceDimensions.size();
    }

    AVFrame* frame = retrieveAVFrame(frameSize, static_cast<int>(AV_PIX_FMT_YUV420P));

    if(frame)
    {
        vpx_image* ret = new vpx_image {};
        memset(ret, 0, sizeof(vpx_image));

        ret->w = ret->d_w = frameSize.width();
        ret->h = ret->d_h = frameSize.height();
        ret->fmt = VPX_IMG_FMT_I420;
        ret->planes[0] = frame->data[0];
        ret->planes[1] = frame->data[1];
        ret->planes[2] = frame->data[2];
        ret->planes[3] = nullptr;
        ret->stride[0] = frame->linesize[0];
        ret->stride[1] = frame->linesize[1];
        ret->stride[2] = frame->linesize[2];
        ret->stride[3] = frame->linesize[3];

        frameLock.unlock();
        return ret;
    }

    // VideoFrame does not contain an AVFrame to spec, generate one here
    frame = generateAVFrame(frameSize, static_cast<int>(AV_PIX_FMT_YUV420P));

    /*
     * We need to "upgrade" the lock to a write lock so we can update our frameBuffer map.
     *
     * It doesn't matter if another thread obtains the write lock before we finish since it is
     * likely writing to somewhere else. Absolute worst-case scenario, we merely perform the
     * generation process twice, and discard the old result.
     */
    frameLock.unlock();
    frameLock.lockForWrite();

    storeAVFrame(frame, frameSize, static_cast<int>(AV_PIX_FMT_YUV420P));

    vpx_image* ret = new vpx_image {};
    memset(ret, 0, sizeof(vpx_image));

    ret->w = ret->d_w = frameSize.width();
    ret->h = ret->d_h = frameSize.height();
    ret->fmt = VPX_IMG_FMT_I420;
    ret->planes[0] = frame->data[0];
    ret->planes[1] = frame->data[1];
    ret->planes[2] = frame->data[2];
    ret->planes[3] = nullptr;
    ret->stride[0] = frame->linesize[0];
    ret->stride[1] = frame->linesize[1];
    ret->stride[2] = frame->linesize[2];
    ret->stride[3] = frame->linesize[3];

    frameLock.unlock();
    return ret;
}

AVFrame* VideoFrame::retrieveAVFrame(const QSize& dimensions, const int pixelFormat)
{
    if(frameBuffer.contains(pixelFormat) && frameBuffer[pixelFormat].contains(dimensionsToKey(dimensions)))
    {
        return frameBuffer[pixelFormat][dimensionsToKey(dimensions)];
    }
    else
    {
        return nullptr;
    }
}

AVFrame* VideoFrame::generateAVFrame(const QSize& dimensions, const int pixelFormat)
{
    AVFrame* ret = av_frame_alloc();

    if(!ret){
        return nullptr;
    }

    // Populate AVFrame fields
    ret->width = dimensions.width();
    ret->height = dimensions.height();
    ret->format = pixelFormat;

    int bufSize = av_image_alloc(ret->data, ret->linesize,
                                 dimensions.width(), dimensions.height(),
                                 static_cast<AVPixelFormat>(pixelFormat), frameAlignment);

    if(bufSize < 0){
        av_frame_free(&ret);
        return nullptr;
    }

    // Bilinear is better for shrinking, bicubic better for upscaling
    int resizeAlgo = sourceDimensions.width() > dimensions.width() ? SWS_BILINEAR : SWS_BICUBIC;

    SwsContext* swsCtx =  sws_getContext(sourceDimensions.width(), sourceDimensions.height(),
                                         static_cast<AVPixelFormat>(sourcePixelFormat),
                                         dimensions.width(), dimensions.height(),
                                         static_cast<AVPixelFormat>(pixelFormat),
                                         resizeAlgo, nullptr, nullptr, nullptr);

    if(!swsCtx){
        av_freep(&ret->data[0]);
        av_frame_unref(ret);
        av_frame_free(&ret);
        return nullptr;
    }

    AVFrame* source = frameBuffer[sourcePixelFormat][dimensionsToKey(sourceDimensions.size())];

    sws_scale(swsCtx, source->data, source->linesize, 0, sourceDimensions.height(), ret->data, ret->linesize);
    sws_freeContext(swsCtx);

    return ret;
}

void VideoFrame::storeAVFrame(AVFrame* frame, const QSize& dimensions, const int pixelFormat)
{
    // We check the prescence of the frame in case of double-computation
    if(frameBuffer.contains(pixelFormat) && frameBuffer[pixelFormat].contains(dimensionsToKey(dimensions)))
    {
        AVFrame* old_ret = frameBuffer[pixelFormat][dimensionsToKey(dimensions)];

        // Free old frame
        av_freep(&old_ret->data[0]);
        av_frame_unref(old_ret);
        av_frame_free(&old_ret);
    }

    frameBuffer[pixelFormat].insert(dimensionsToKey(dimensions), frame);
}

void VideoFrame::deleteFrameBuffer()
{
    const quint64 sourceKey = dimensionsToKey(sourceDimensions.size());

    for(auto pixFmtIter = frameBuffer.begin(); pixFmtIter != frameBuffer.end(); ++pixFmtIter)
    {
        const auto& pixFmtMap = pixFmtIter.value();

        for(auto dimKeyIter = pixFmtMap.begin(); dimKeyIter != pixFmtMap.end(); ++dimKeyIter)
        {
            AVFrame* frame = dimKeyIter.value();

            // Treat source frame and derived frames separately
            if(pixFmtIter.key() == sourcePixelFormat && dimKeyIter.key() == sourceKey)
            {
                if(freeSourceFrame)
                {
                    av_freep(&frame->data[0]);
                }
                av_frame_unref(frame);
                av_frame_free(&frame);
            }
            else
            {
                av_freep(&frame->data[0]);
                av_frame_unref(frame);
                av_frame_free(&frame);
            }
        }
    }

    frameBuffer.clear();
}
