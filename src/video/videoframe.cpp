/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

extern "C" {
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
 * @var VideoFrame::dataAlignment
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
VideoFrame::AtomicIDType VideoFrame::frameIDs{0};

std::unordered_map<VideoFrame::IDType, QMutex> VideoFrame::mutexMap{};
std::unordered_map<VideoFrame::IDType, std::unordered_map<VideoFrame::IDType, std::weak_ptr<VideoFrame>>>
    VideoFrame::refsMap{};

QReadWriteLock VideoFrame::refsLock{};

/**
 * @brief Constructs a new instance of a VideoFrame, sourced by a given AVFrame pointer.
 *
 * @param sourceID the VideoSource's ID to track the frame under.
 * @param sourceFrame the source AVFrame pointer to use, must be valid.
 * @param dimensions the dimensions of the AVFrame, obtained from the AVFrame if not given.
 * @param pixFmt the pixel format of the AVFrame, obtained from the AVFrame if not given.
 * @param freeSourceFrame whether to free the source frame buffers or not.
 */
VideoFrame::VideoFrame(IDType sourceID_, AVFrame* sourceFrame, QRect dimensions, int pixFmt,
                       bool freeSourceFrame_)
    : frameID(frameIDs++)
    , sourceID(sourceID_)
    , sourceDimensions(dimensions)
    , sourceFrameKey(getFrameKey(dimensions.size(), pixFmt, sourceFrame->linesize[0]))
    , freeSourceFrame(freeSourceFrame_)
{

    // We override the pixel format in the case a deprecated one is used
    switch (pixFmt) {
    case AV_PIX_FMT_YUVJ420P: {
        sourcePixelFormat = AV_PIX_FMT_YUV420P;
        sourceFrame->color_range = AVCOL_RANGE_MPEG;
        break;
    }

    case AV_PIX_FMT_YUVJ411P: {
        sourcePixelFormat = AV_PIX_FMT_YUV411P;
        sourceFrame->color_range = AVCOL_RANGE_MPEG;
        break;
    }

    case AV_PIX_FMT_YUVJ422P: {
        sourcePixelFormat = AV_PIX_FMT_YUV422P;
        sourceFrame->color_range = AVCOL_RANGE_MPEG;
        break;
    }

    case AV_PIX_FMT_YUVJ444P: {
        sourcePixelFormat = AV_PIX_FMT_YUV444P;
        sourceFrame->color_range = AVCOL_RANGE_MPEG;
        break;
    }

    case AV_PIX_FMT_YUVJ440P: {
        sourcePixelFormat = AV_PIX_FMT_YUV440P;
        sourceFrame->color_range = AVCOL_RANGE_MPEG;
        break;
    }

    default: {
        sourcePixelFormat = pixFmt;
        sourceFrame->color_range = AVCOL_RANGE_UNSPECIFIED;
    }
    }

    frameBuffer[sourceFrameKey] = sourceFrame;
}

VideoFrame::VideoFrame(IDType sourceID_, AVFrame* sourceFrame, bool freeSourceFrame_)
    : VideoFrame(sourceID_, sourceFrame, QRect{0, 0, sourceFrame->width, sourceFrame->height},
                 sourceFrame->format, freeSourceFrame_)
{
}

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

    if (refsMap.count(sourceID) > 0) {
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

    if (refsMap.count(sourceID) == 0) {
        // We need to add a new source to our reference map, obtain write lock
        refsLock.unlock();
        refsLock.lockForWrite();
    }

    QMutex& sourceMutex = mutexMap[sourceID];

    sourceMutex.lock();

    std::shared_ptr<VideoFrame> ret{this};

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

    if (refsMap.count(sourceID) == 0) {
        // No tracking reference exists for source, simply return
        refsLock.unlock();

        return;
    }

    if (releaseFrames) {
        QMutex& sourceMutex = mutexMap[sourceID];

        sourceMutex.lock();

        for (auto& frameIterator : refsMap[sourceID]) {
            std::shared_ptr<VideoFrame> frame = frameIterator.second.lock();

            if (frame) {
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
 * @param frameSize the dimensions of the frame to get. Defaults to source frame size if frameSize
 * is invalid.
 * @param pixelFormat the desired pixel format of the frame.
 * @param requireAligned true if the returned frame must be frame aligned, false if not.
 * @return a pointer to a AVFrame with the given parameters or nullptr if the VideoFrame is no
 * longer valid.
 */
const AVFrame* VideoFrame::getAVFrame(QSize frameSize, const int pixelFormat, const bool requireAligned)
{
    if (!frameSize.isValid()) {
        frameSize = sourceDimensions.size();
    }

    // Since we are retrieving the AVFrame* directly, we merely need to pass the arguement through
    const std::function<AVFrame*(AVFrame * const)> converter = [](AVFrame* const frame) {
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
 * @param frameSize the given frame size of QImage to generate. Defaults to source frame size if
 * frameSize is invalid.
 * @return a QImage that represents this VideoFrame, sharing it's buffers or a null image if
 * this VideoFrame is no longer valid.
 */
QImage VideoFrame::toQImage(QSize frameSize)
{
    if (!frameSize.isValid()) {
        frameSize = sourceDimensions.size();
    }

    // Converter function (constructs QImage out of AVFrame*)
    const std::function<QImage(AVFrame * const)> converter = [&](AVFrame* const frame) {
        return QImage{*(frame->data), frameSize.width(), frameSize.height(), *(frame->linesize),
                      QImage::Format_RGB888};
    };

    // Returns an empty constructed QImage in case of invalid generation
    return toGenericObject(frameSize, AV_PIX_FMT_RGB24, false, converter, QImage{});
}

/**
 * @brief Converts this VideoFrame to a ToxAVFrame that shares this VideoFrame's buffer.
 *
 * The given ToxAVFrame will be frame aligned under a pixel format of planar YUV with a chroma
 * subsampling format of 4:2:0 (i.e. AV_PIX_FMT_YUV420P).
 *
 * @param frameSize the given frame size of ToxAVFrame to generate. Defaults to source frame size
 * if frameSize is invalid.
 * @return a ToxAVFrame structure that represents this VideoFrame, sharing it's buffers or an
 * empty structure if this VideoFrame is no longer valid.
 */
ToxYUVFrame VideoFrame::toToxYUVFrame(QSize frameSize)
{
    if (!frameSize.isValid()) {
        frameSize = sourceDimensions.size();
    }

    // Converter function (constructs ToxAVFrame out of AVFrame*)
    const std::function<ToxYUVFrame(AVFrame * const)> converter = [&](AVFrame* const frame) {
        ToxYUVFrame ret{static_cast<std::uint16_t>(frameSize.width()),
                        static_cast<std::uint16_t>(frameSize.height()), frame->data[0],
                        frame->data[1], frame->data[2]};

        return ret;
    };

    return toGenericObject(frameSize, AV_PIX_FMT_YUV420P, true, converter,
                           ToxYUVFrame{0, 0, nullptr, nullptr, nullptr});
}

/**
 * @brief Returns the ID for the given frame.
 *
 * Frame IDs are globally unique (with respect to the running instance).
 *
 * @return an integer representing the ID of this frame.
 */
VideoFrame::IDType VideoFrame::getFrameID() const
{
    return frameID;
}

/**
 * @brief Returns the ID for the VideoSource which created this frame.
 *
 * @return an integer representing the ID of the VideoSource which created this frame.
 */
VideoFrame::IDType VideoFrame::getSourceID() const
{
    return sourceID;
}

/**
 * @brief Retrieves a copy of the source VideoFrame's dimensions.
 *
 * @return QRect copy representing the source VideoFrame's dimensions.
 */
QRect VideoFrame::getSourceDimensions() const
{
    return sourceDimensions;
}

/**
 * @brief Retrieves a copy of the source VideoFormat's pixel format.
 *
 * @return integer copy representing the source VideoFrame's pixel format.
 */
int VideoFrame::getSourcePixelFormat() const
{
    return sourcePixelFormat;
}


/**
 * @brief Constructs a new FrameBufferKey with the given attributes.
 *
 * @param width the width of the frame.
 * @param height the height of the frame.
 * @param pixFmt the pixel format of the frame.
 * @param lineAligned whether the linesize matches the width of the image.
 */
VideoFrame::FrameBufferKey::FrameBufferKey(const int width, const int height, const int pixFmt,
                                           const bool lineAligned)
    : frameWidth(width)
    , frameHeight(height)
    , pixelFormat(pixFmt)
    , linesizeAligned(lineAligned)
{
}

/**
 * @brief Comparison operator for FrameBufferKey.
 *
 * @param other instance to compare against.
 * @return true if instances are equivilent, false otherwise.
 */
bool VideoFrame::FrameBufferKey::operator==(const FrameBufferKey& other) const
{
    return pixelFormat == other.pixelFormat && frameWidth == other.frameWidth
           && frameHeight == other.frameHeight && linesizeAligned == other.linesizeAligned;
}

/**
 * @brief Not equal to operator for FrameBufferKey.
 *
 * @param other instance to compare against
 * @return true if instances are not equivilent, false otherwise.
 */
bool VideoFrame::FrameBufferKey::operator!=(const FrameBufferKey& other) const
{
    return !operator==(other);
}

/**
 * @brief Hash function for FrameBufferKey.
 *
 * This function computes a hash value for use with std::unordered_map.
 *
 * @param key the given instance to compute hash value of.
 * @return the hash of the given instance.
 */
size_t VideoFrame::FrameBufferKey::hash(const FrameBufferKey& key)
{
    std::hash<int> intHasher;
    std::hash<bool> boolHasher;

    // Use java-style hash function to combine fields
    // See: https://en.wikipedia.org/wiki/Java_hashCode%28%29#hashCode.28.29_in_general

    size_t ret = 47;

    ret = 37 * ret + intHasher(key.frameWidth);
    ret = 37 * ret + intHasher(key.frameHeight);
    ret = 37 * ret + intHasher(key.pixelFormat);
    ret = 37 * ret + boolHasher(key.linesizeAligned);

    return ret;
}

/**
 * @brief Generates a key object based on given parameters.
 *
 * @param frameSize the given size of the frame.
 * @param pixFmt the pixel format of the frame.
 * @param linesize the maximum linesize of the frame, may be larger than the width.
 * @return a FrameBufferKey object representing the key for the frameBuffer map.
 */
VideoFrame::FrameBufferKey VideoFrame::getFrameKey(const QSize& frameSize, const int pixFmt,
                                                   const int linesize)
{
    return getFrameKey(frameSize, pixFmt, frameSize.width() == linesize);
}

/**
 * @brief Generates a key object based on given parameters.
 *
 * @param frameSize the given size of the frame.
 * @param pixFmt the pixel format of the frame.
 * @param frameAligned true if the frame is aligned, false otherwise.
 * @return a FrameBufferKey object representing the key for the frameBuffer map.
 */
VideoFrame::FrameBufferKey VideoFrame::getFrameKey(const QSize& frameSize, const int pixFmt,
                                                   const bool frameAligned)
{
    return {frameSize.width(), frameSize.height(), pixFmt, frameAligned};
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
 * @param dimensions the dimensions of the frame, must be valid.
 * @param pixelFormat the desired pixel format of the frame.
 * @param requireAligned true if the frame must be frame aligned, false otherwise.
 * @return a pointer to a AVFrame with the given parameters or nullptr if no such frame was
 * found.
 */
AVFrame* VideoFrame::retrieveAVFrame(const QSize& dimensions, const int pixelFormat,
                                     const bool requireAligned)
{
    if (!requireAligned) {
        /*
         * We attempt to obtain a unaligned frame first because an unaligned linesize corresponds
         * to a data aligned frame.
         */
        FrameBufferKey frameKey = getFrameKey(dimensions, pixelFormat, false);

        if (frameBuffer.count(frameKey) > 0) {
            return frameBuffer[frameKey];
        }
    }

    FrameBufferKey frameKey = getFrameKey(dimensions, pixelFormat, true);

    if (frameBuffer.count(frameKey) > 0) {
        return frameBuffer[frameKey];
    } else {
        return nullptr;
    }
}

/**
 * @brief Generates an AVFrame based on the given specifications.
 *
 * This function is not thread-safe and must be called from a thread-safe context.
 *
 * @param dimensions the required dimensions for the frame, must be valid.
 * @param pixelFormat the required pixel format for the frame.
 * @param requireAligned true if the generated frame needs to be frame aligned, false otherwise.
 * @return an AVFrame with the given specifications.
 */
AVFrame* VideoFrame::generateAVFrame(const QSize& dimensions, const int pixelFormat,
                                     const bool requireAligned)
{
    AVFrame* ret = av_frame_alloc();

    if (!ret) {
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

    const bool alreadyAligned = dimensions.width() % dataAlignment == 0 && dimensions.height() % dataAlignment == 0;

    if (!requireAligned || alreadyAligned) {
        bufSize = av_image_alloc(ret->data, ret->linesize, dimensions.width(), dimensions.height(),
                                 static_cast<AVPixelFormat>(pixelFormat), dataAlignment);
    } else {
        bufSize = av_image_alloc(ret->data, ret->linesize, dimensions.width(), dimensions.height(),
                                 static_cast<AVPixelFormat>(pixelFormat), 1);
    }

    if (bufSize < 0) {
        av_frame_free(&ret);
        return nullptr;
    }

    // Bilinear is better for shrinking, bicubic better for upscaling
    int resizeAlgo = sourceDimensions.width() > dimensions.width() ? SWS_BILINEAR : SWS_BICUBIC;

    SwsContext* swsCtx =
        sws_getContext(sourceDimensions.width(), sourceDimensions.height(),
                       static_cast<AVPixelFormat>(sourcePixelFormat), dimensions.width(),
                       dimensions.height(), static_cast<AVPixelFormat>(pixelFormat), resizeAlgo,
                       nullptr, nullptr, nullptr);

    if (!swsCtx) {
        av_freep(&ret->data[0]);
#if LIBAVCODEC_VERSION_INT < 3747941
        av_frame_unref(ret);
#endif
        av_frame_free(&ret);
        return nullptr;
    }

    AVFrame* source = frameBuffer[sourceFrameKey];

    sws_scale(swsCtx, source->data, source->linesize, 0, sourceDimensions.height(), ret->data,
              ret->linesize);
    sws_freeContext(swsCtx);

    return ret;
}

/**
 * @brief Stores a given AVFrame within the frameBuffer map.
 *
 * As protection against duplicate frames, the storage mechanism will only allow one frame of a
 * given type to exist in the frame buffer. Should the given frame type already exist in the frame
 * buffer, the given frame will be freed and have it's buffers invalidated. In order to ensure
 * correct operation, always replace the frame pointer with the one returned by this function.
 *
 * As an example:
 * @code{.cpp}
 * AVFrame* frame = // create AVFrame...
 *
 * frame = storeAVFrame(frame, dimensions, pixelFormat);
 * @endcode
 *
 * This function is not thread-safe and must be called from a thread-safe context.
 *
 * @param frame the given frame to store.
 * @param dimensions the dimensions of the frame, must be valid.
 * @param pixelFormat the pixel format of the frame.
 * @return The given AVFrame* or a pre-existing AVFrame* that already exists in the frameBuffer.
 */
AVFrame* VideoFrame::storeAVFrame(AVFrame* frame, const QSize& dimensions, const int pixelFormat)
{
    FrameBufferKey frameKey = getFrameKey(dimensions, pixelFormat, frame->linesize[0]);

    // We check the prescence of the frame in case of double-computation
    if (frameBuffer.count(frameKey) > 0) {
        AVFrame* old_ret = frameBuffer[frameKey];

        // Free new frame
        av_freep(&frame->data[0]);
#if LIBAVCODEC_VERSION_INT < 3747941
        av_frame_unref(frame);
#endif
        av_frame_free(&frame);

        return old_ret;
    } else {
        frameBuffer[frameKey] = frame;

        return frame;
    }
}

/**
 * @brief Releases all frames within the frame buffer.
 *
 * This function is not thread-safe and must be called from a thread-safe context.
 */
void VideoFrame::deleteFrameBuffer()
{
    // An empty framebuffer represents a frame that's already been freed
    if (frameBuffer.empty()) {
        return;
    }

    for (const auto& frameIterator : frameBuffer) {
        AVFrame* frame = frameIterator.second;

        // Treat source frame and derived frames separately
        if (sourceFrameKey == frameIterator.first) {
            if (freeSourceFrame) {
                av_freep(&frame->data[0]);
            }
#if LIBAVCODEC_VERSION_INT < 3747941
            av_frame_unref(frame);
#endif
            av_frame_free(&frame);
        } else {
            av_freep(&frame->data[0]);
#if LIBAVCODEC_VERSION_INT < 3747941
            av_frame_unref(frame);
#endif
            av_frame_free(&frame);
        }
    }

    frameBuffer.clear();
}

/**
 * @brief Converts this VideoFrame to a generic type T based on the given parameters and
 * supplied converter functions.
 *
 * This function is used internally to create various toXObject functions that all follow the
 * same generation pattern (where XObject is some existing type like QImage).
 *
 * In order to create such a type, a object constructor function is required that takes the
 * generated AVFrame object and creates type T out of it. This function additionally requires
 * a null object of type T that represents an invalid/null object for when the generation
 * process fails (e.g. when the VideoFrame is no longer valid).
 *
 * @param dimensions the dimensions of the frame, must be valid.
 * @param pixelFormat the pixel format of the frame.
 * @param requireAligned true if the generated frame needs to be frame aligned, false otherwise.
 * @param objectConstructor a std::function that takes the generated AVFrame and converts it
 * to an object of type T.
 * @param nullObject an object of type T that represents the null/invalid object to be used
 * when the generation process fails.
 */
template <typename T>
T VideoFrame::toGenericObject(const QSize& dimensions, const int pixelFormat, const bool requireAligned,
                              const std::function<T(AVFrame* const)>& objectConstructor,
                              const T& nullObject)
{
    frameLock.lockForRead();

    // We return nullObject if the VideoFrame is no longer valid
    if (frameBuffer.size() == 0) {
        frameLock.unlock();
        return nullObject;
    }

    AVFrame* frame = retrieveAVFrame(dimensions, static_cast<int>(pixelFormat), requireAligned);

    if (frame) {
        T ret = objectConstructor(frame);

        frameLock.unlock();
        return ret;
    }

    // VideoFrame does not contain an AVFrame to spec, generate one here
    frame = generateAVFrame(dimensions, static_cast<int>(pixelFormat), requireAligned);

    /*
     * We need to "upgrade" the lock to a write lock so we can update our frameBuffer map.
     *
     * It doesn't matter if another thread obtains the write lock before we finish since it is
     * likely writing to somewhere else. Worst-case scenario, we merely perform the generation
     * process twice, and discard the old result.
     */
    frameLock.unlock();
    frameLock.lockForWrite();

    frame = storeAVFrame(frame, dimensions, static_cast<int>(pixelFormat));

    T ret = objectConstructor(frame);

    frameLock.unlock();
    return ret;
}

// Explicitly specialize VideoFrame::toGenericObject() function
template QImage VideoFrame::toGenericObject<QImage>(
    const QSize& dimensions, const int pixelFormat, const bool requireAligned,
    const std::function<QImage(AVFrame* const)> &objectConstructor, const QImage& nullObject);
template ToxYUVFrame VideoFrame::toGenericObject<ToxYUVFrame>(
    const QSize& dimensions, const int pixelFormat, const bool requireAligned,
    const std::function<ToxYUVFrame(AVFrame* const)> &objectConstructor, const ToxYUVFrame& nullObject);

/**
 * @brief Returns whether the given ToxYUVFrame represents a valid frame or not.
 *
 * Valid frames are frames in which both width and height are greater than zero.
 *
 * @return true if the frame is valid, false otherwise.
 */
bool ToxYUVFrame::isValid() const
{
    return width > 0 && height > 0;
}

/**
 * @brief Checks if the given ToxYUVFrame is valid or not, delegates to isValid().
 */
ToxYUVFrame::operator bool() const
{
    return isValid();
}
