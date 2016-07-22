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

// Initialize static fields
VideoFrame::AtomicIDType VideoFrame::frameIDs {0};

std::unordered_map<VideoFrame::IDType, QMutex> VideoFrame::mutexMap {};
std::unordered_map<VideoFrame::IDType, std::unordered_map<VideoFrame::IDType, std::weak_ptr<VideoFrame>>> VideoFrame::refsMap {};

QReadWriteLock VideoFrame::refsLock {};

VideoFrame::VideoFrame(IDType sourceID, AVFrame* sourceFrame, QRect dimensions, int pixFmt, bool freeSourceFrame)
    : frameID(frameIDs.fetch_add(std::memory_order_relaxed)),
      sourceID(sourceID),
      sourceDimensions(dimensions),
      sourcePixelFormat(pixFmt),
      sourceFrameKey(getFrameKey(dimensions.size(), pixFmt, sourceFrame->linesize[0])),
      freeSourceFrame(freeSourceFrame)
{
    frameBuffer[sourceFrameKey] = sourceFrame;
}

VideoFrame::VideoFrame(IDType sourceID, AVFrame* sourceFrame, bool freeSourceFrame)
    : VideoFrame(sourceID, sourceFrame, QRect {0, 0, sourceFrame->width, sourceFrame->height}, sourceFrame->format, freeSourceFrame){}

VideoFrame::~VideoFrame()
{
    // Release frame
    frameLock.lockForWrite();

    deleteFrameBuffer();

    frameLock.unlock();

    // Delete tracked reference
    refsLock.lockForRead();

    if(refsMap.count(sourceID) > 0)
    {
        QMutex& sourceMutex = mutexMap[sourceID];

        sourceMutex.lock();
        refsMap[sourceID].erase(frameID);
        sourceMutex.unlock();
    }

    refsLock.unlock();
}

bool VideoFrame::isValid()
{
    frameLock.lockForRead();
    bool retValue = frameBuffer.size() > 0;
    frameLock.unlock();

    return retValue;
}

std::shared_ptr<VideoFrame> VideoFrame::trackFrame()
{
    // Add frame to tracked reference list
    refsLock.lockForRead();

    if(refsMap.count(sourceID) == 0)
    {
        // We need to add a new source to our reference map, obtain write lock
        refsLock.unlock();
        refsLock.lockForWrite();
    }

    QMutex& sourceMutex = mutexMap[sourceID];

    sourceMutex.lock();

    std::shared_ptr<VideoFrame> ret {this};

    refsMap[sourceID][frameID] = ret;

    sourceMutex.unlock();
    refsLock.unlock();

    return ret;
}

void VideoFrame::untrackFrames(const VideoFrame::IDType& sourceID, bool releaseFrames)
{
    refsLock.lockForWrite();

    if(refsMap.count(sourceID) == 0)
    {
        // No tracking reference exists for source, simply return
        refsLock.unlock();

        return;
    }

    if(releaseFrames)
    {
        QMutex& sourceMutex = mutexMap[sourceID];

        sourceMutex.lock();

        for(auto& frameIterator : refsMap[sourceID])
        {
            std::shared_ptr<VideoFrame> frame = frameIterator.second.lock();

            if(frame)
            {
                frame->releaseFrame();
            }
        }

        sourceMutex.unlock();
    }

    refsMap[sourceID].clear();

    mutexMap.erase(sourceID);
    refsMap.erase(sourceID);

    refsLock.unlock();
}

void VideoFrame::releaseFrame()
{
    frameLock.lockForWrite();

    deleteFrameBuffer();

    frameLock.unlock();
}

const AVFrame* VideoFrame::getAVFrame(QSize frameSize, const int pixelFormat, const bool requireAligned)
{
    // Since we are retrieving the AVFrame* directly, we merely need to pass the arguement through
    const std::function<AVFrame*(AVFrame* const)> converter = [](AVFrame* const frame)
    {
        return frame;
    };

    // We need an explicit null pointer holding object to pass to toGenericObject()
    AVFrame* nullPointer = nullptr;

    // Returns std::nullptr case of invalid generation
    return toGenericObject(frameSize, pixelFormat, requireAligned, converter, nullPointer);
}

QImage VideoFrame::toQImage(QSize frameSize)
{
    if(frameSize.width() == 0 && frameSize.height() == 0)
    {
        frameSize = sourceDimensions.size();
    }

    // Converter function (constructs QImage out of AVFrame*)
    const std::function<QImage(AVFrame* const)> converter = [&](AVFrame* const frame)
    {
        return QImage {*(frame->data), frameSize.width(), frameSize.height(), *(frame->linesize), QImage::Format_RGB888};
    };

    // Returns an empty constructed QImage in case of invalid generation
    return toGenericObject(frameSize, AV_PIX_FMT_RGB24, false, converter, QImage {});
}

ToxYUVFrame VideoFrame::toToxAVFrame(QSize frameSize)
{
    if(frameSize.width() == 0 && frameSize.height() == 0)
    {
        frameSize = sourceDimensions.size();
    }

    // Converter function (constructs ToxAVFrame out of AVFrame*)
    const std::function<ToxYUVFrame(AVFrame* const)> converter = [&](AVFrame* const frame)
    {
        ToxYUVFrame ret
        {
            static_cast<std::uint16_t>(frameSize.width()),
            static_cast<std::uint16_t>(frameSize.height()),
            frame->data[0], frame->data[1], frame->data[2]
        };

        return ret;
    };

    return toGenericObject(frameSize, AV_PIX_FMT_YUV420P, true, converter, ToxYUVFrame {0, 0, nullptr, nullptr, nullptr});
}

AVFrame* VideoFrame::retrieveAVFrame(const QSize& dimensions, const int pixelFormat, const bool requireAligned)
{
    if(!requireAligned)
    {
        /*
         * We attempt to obtain a unaligned frame first because an unaligned linesize corresponds
         * to a data aligned frame.
         */
        FrameBufferKey frameKey = getFrameKey(dimensions, pixelFormat, false);

        if(frameBuffer.count(frameKey) > 0)
        {
            return frameBuffer[frameKey];
        }
    }

    FrameBufferKey frameKey = getFrameKey(dimensions, pixelFormat, true);

    if(frameBuffer.count(frameKey) > 0)
    {
        return frameBuffer[frameKey];
    }
    else
    {
        return nullptr;
    }
}

AVFrame* VideoFrame::generateAVFrame(const QSize& dimensions, const int pixelFormat, const bool requireAligned)
{
    AVFrame* ret = av_frame_alloc();

    if(!ret){
        return nullptr;
    }

    // Populate AVFrame fields
    ret->width = dimensions.width();
    ret->height = dimensions.height();
    ret->format = pixelFormat;

    /*
     * We generate a frame under data alignment only if the dimensions allow us to be frame aligned
     * or if the caller doesn't require frame alignment
     */

    int bufSize;

    if(!requireAligned || (dimensions.width() % 8 == 0 && dimensions.height() % 8 == 0))
    {
        bufSize = av_image_alloc(ret->data, ret->linesize,
                                 dimensions.width(), dimensions.height(),
                                 static_cast<AVPixelFormat>(pixelFormat), dataAlignment);
    }
    else
    {
        bufSize = av_image_alloc(ret->data, ret->linesize,
                                 dimensions.width(), dimensions.height(),
                                 static_cast<AVPixelFormat>(pixelFormat), 1);
    }

    if(bufSize < 0)
    {
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

    AVFrame* source = frameBuffer[sourceFrameKey];

    sws_scale(swsCtx, source->data, source->linesize, 0, sourceDimensions.height(), ret->data, ret->linesize);
    sws_freeContext(swsCtx);

    return ret;
}

void VideoFrame::storeAVFrame(AVFrame* frame, const QSize& dimensions, const int pixelFormat)
{
    FrameBufferKey frameKey = getFrameKey(dimensions, pixelFormat, frame->linesize[0]);

    // We check the prescence of the frame in case of double-computation
    if(frameBuffer.count(frameKey) > 0)
    {
        AVFrame* old_ret = frameBuffer[frameKey];

        // Free old frame
        av_freep(&old_ret->data[0]);
        av_frame_unref(old_ret);
        av_frame_free(&old_ret);
    }

    frameBuffer[frameKey] = frame;
}

void VideoFrame::deleteFrameBuffer()
{
    for(const auto& frameIterator : frameBuffer)
    {
        AVFrame* frame = frameIterator.second;

        // Treat source frame and derived frames separately
        if(sourceFrameKey == frameIterator.first)
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

    frameBuffer.clear();
}

VideoFrame::FrameBufferKey::FrameBufferKey(const int pixFmt, const int width, const int height, const bool lineAligned)
    : frameWidth(width),
      frameHeight(height),
      pixelFormat(pixFmt),
      linesizeAligned(lineAligned){}
