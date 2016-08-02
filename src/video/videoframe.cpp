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

/**
 * @struct ToxYUVFrame
 * @brief A simple structure to represent a ToxYUV video frame (corresponds to a frame encoded
 * under format: AV_PIX_FMT_YUV420P [FFmpeg] or VPX_IMG_FMT_I420 [WebM]).
 *
 * This structure exists for convenience and code clarity when ferrying YUV420 frames from one
 * source to another. The buffers pointed to by the struct should not be owned by the struct nor
 * should they be freed from the struct, instead this struct functions only as a simple alias to a
 * more complicated frame container like AVFrame.
 *
 * The creation of this structure was done to replace existing code which mis-used vpx_image
 * structs when passing frame data to toxcore.
 *
 *
 * @class VideoFrame
 * @brief An ownernship and management class for AVFrames.
 *
 * VideoFrame takes ownership of an AVFrame* and allows fast conversions to other formats.
 * Ownership of all video frame buffers is kept by the VideoFrame, even after conversion. All
 * references to the frame data become invalid when the VideoFrame is deleted. We try to avoid
 * pixel format conversions as much as possible, at the cost of some memory.
 *
 * Every function in this class is thread safe apart from concurrent construction and deletion of
 * the object.
 *
 * This class uses the phrase "frame alignment" to specify the property that each frame's width is
 * equal to it's maximum linesize. Note: this is NOT "data alignment" which specifies how allocated
 * buffers are aligned in memory. Though internally the two are related, unless otherwise specified
 * all instances of the term "alignment" exposed from public functions refer to frame alignment.
 *
 * Frame alignment is an important concept because ToxAV does not support frames with linesizes not
 * directly equal to the width.
 *
 *
 * @var dataAlignment
 * @brief Data alignment parameter used to populate AVFrame buffers.
 *
 * This field is public in effort to standardize the data alignment parameter for all AVFrame
 * allocations.
 *
 * It's currently set to 32-byte alignment for AVX2 support.
 *
 *
 * @class FrameBufferKey
 * @brief A class representing a structure that stores frame properties to be used as the key
 * value for a std::unordered_map.
 */

// Initialize static fields
VideoFrame::AtomicIDType VideoFrame::frameIDs {0};

std::unordered_map<VideoFrame::IDType, QMutex> VideoFrame::mutexMap {};
std::unordered_map<VideoFrame::IDType, std::unordered_map<VideoFrame::IDType, std::weak_ptr<VideoFrame>>> VideoFrame::refsMap {};

QReadWriteLock VideoFrame::refsLock {};

/**
 * @brief Constructs a new instance of a VideoFrame, sourced by a given AVFrame pointer.
 * 
 * @param sourceID the VideoSource's ID to track the frame under.
 * @param sourceFrame the source AVFrame pointer to use, must be valid.
 * @param dimensions the dimensions of the AVFrame, obtained from the AVFrame if not given.
 * @param pixFmt the pixel format of the AVFrame, obtained from the AVFrame if not given.
 * @param freeSourceFrame whether to free the source frame buffers or not.
 */
VideoFrame::VideoFrame(IDType sourceID, AVFrame* sourceFrame, QRect dimensions, int pixFmt, bool freeSourceFrame)
    : frameID(frameIDs++),
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

/**
 * @brief Destructor for VideoFrame.
 */
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

/**
 * @brief Returns the validity of this VideoFrame.
 *
 * A VideoFrame is valid if it manages at least one AVFrame. A VideoFrame can be invalidated
 * by calling releaseFrame() on it.
 *
 * @return true if the VideoFrame is valid, false otherwise.
 */
bool VideoFrame::isValid()
{
    frameLock.lockForRead();
    bool retValue = frameBuffer.size() > 0;
    frameLock.unlock();

    return retValue;
}

/**
 * @brief Causes the VideoFrame class to maintain an internal reference for the frame.
 *
 * The internal reference is managed via a std::weak_ptr such that it doesn't inhibit
 * destruction of the object once all external references are no longer reachable.
 *
 * @return a std::shared_ptr holding a reference to this frame.
 */
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

/**
 * @brief Untracks all the frames for the given VideoSource, releasing them if specified.
 *
 * This function causes all internally tracked frames for the given VideoSource to be dropped.
 * If the releaseFrames option is set to true, the frames are sequentially released on the
 * caller's thread in an unspecified order.
 *
 * @param sourceID the ID of the VideoSource to untrack frames from.
 * @param releaseFrames true to release the frames as necessary, false otherwise. Defaults to
 * false.
 */
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

/**
 * @brief Releases all frames managed by this VideoFrame and invalidates it.
 */
void VideoFrame::releaseFrame()
{
    frameLock.lockForWrite();

    deleteFrameBuffer();

    frameLock.unlock();
}

/**
 * @brief Retrieves an AVFrame derived from the source based on the given parameters.
 *
 * If a given frame does not exist, this function will perform appropriate conversions to
 * return a frame that fulfills the given parameters.
 *
 * @param frameSize the dimensions of the frame to get. If frame size is 0, defaults to source
 * frame size.
 * @param pixelFormat the desired pixel format of the frame.
 * @param requireAligned true if the returned frame must be frame aligned, false if not.
 * @return a pointer to a AVFrame with the given parameters or nullptr if the VideoFrame is no
 * longer valid.
 */
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

/**
 * @brief Converts this VideoFrame to a QImage that shares this VideoFrame's buffer.
 *
 * The VideoFrame will be scaled into the RGB24 pixel format along with the given
 * dimension.
 *
 * @param frameSize the given frame size of QImage to generate. If frame size is 0, defaults to
 * source frame size.
 * @return a QImage that represents this VideoFrame, sharing it's buffers or a null image if
 * this VideoFrame is no longer valid.
 */
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

/**
 * @brief Converts this VideoFrame to a ToxAVFrame that shares this VideoFrame's buffer.
 *
 * The given ToxAVFrame will be frame aligned under a pixel format of planar YUV with a chroma
 * subsampling format of 4:2:0 (i.e. AV_PIX_FMT_YUV420P).
 *
 * @param frameSize the given frame size of ToxAVFrame to generate. If frame size is 0, defaults
 * to source frame size.
 * @return a ToxAVFrame structure that represents this VideoFrame, sharing it's buffers or an
 * empty structure if this VideoFrame is no longer valid.
 */
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

/**
 * @brief Retrieves an AVFrame derived from the source based on the given parameters without
 * obtaining a lock.
 *
 * This function is not thread-safe and must be called from a thread-safe context.
 *
 * Note: this function differs from getAVFrame() in that it returns a nullptr if no frame was
 * found.
 *
 * @param dimensions the dimensions of the frame.
 * @param pixelFormat the desired pixel format of the frame.
 * @param requireAligned true if the frame must be frame aligned, false otherwise.
 * @return a pointer to a AVFrame with the given parameters or nullptr if no such frame was
 * found.
 */
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

/**
 * @brief Generates an AVFrame based on the given specifications.
 *
 * This function is not thread-safe and must be called from a thread-safe context.
 *
 * @param dimensions the required dimensions for the frame.
 * @param pixelFormat the required pixel format for the frame.
 * @param requireAligned true if the generated frame needs to be frame aligned, false otherwise.
 * @return an AVFrame with the given specifications.
 */
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

/**
 * @brief Stores a given AVFrame within the frameBuffer map.
 *
 * This function is not thread-safe and must be called from a thread-safe context.
 *
 * @param frame the given frame to store.
 * @param dimensions the dimensions of the frame.
 * @param pixelFormat the pixel format of the frame.
 */
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

/**
 * @brief Releases all frames within the frame buffer.
 *
 * This function is not thread-safe and must be called from a thread-safe context.
 */
void VideoFrame::deleteFrameBuffer()
{
    // An empty framebuffer represents a frame that's already been freed
    if(frameBuffer.empty()){
        return;
    }

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

/**
 * @brief Constructs a new FrameBufferKey with the given attributes.
 *
 * @param width the width of the frame.
 * @param height the height of the frame.
 * @param pixFmt the pixel format of the frame.
 * @param lineAligned whether the linesize matches the width of the image.
 */
VideoFrame::FrameBufferKey::FrameBufferKey(const int pixFmt, const int width, const int height, const bool lineAligned)
    : frameWidth(width),
      frameHeight(height),
      pixelFormat(pixFmt),
      linesizeAligned(lineAligned){}
