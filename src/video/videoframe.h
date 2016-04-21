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

#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H

#include <QHash>
#include <QImage>
#include <QReadWriteLock>
#include <QRect>
#include <QSize>

extern "C"{
#include <libavcodec/avcodec.h>
}

#include <vpx/vpx_image.h>
#include <functional>
#include <memory>

/**
 * @brief An ownernship and management class for AVFrames.
 *
 * VideoFrame takes ownership of an AVFrame* and allows fast conversions to other formats.
 * Ownership of all video frame buffers is kept by the VideoFrame, even after conversion. All
 * references to the frame data become invalid when the VideoFrame is deleted. We try to avoid
 * pixel format conversions as much as possible, at the cost of some memory.
 *
 * Every function in this class is thread safe apart from concurrent construction and deletion of
 * the object.
 */
class VideoFrame
{
public:
    /**
     * @brief Constructs a new instance of a VideoFrame, sourced by a given AVFrame pointer.
     * @param sourceFrame the source AVFrame pointer to use, must be valid.
     * @param dimensions the dimensions of the AVFrame, obtained from the AVFrame if not given.
     * @param pixFmt the pixel format of the AVFrame, obtained from the AVFrame if not given.
     * @param destructCallback callback function to run upon destruction of the VideoFrame
     * this callback is only run when destroying a valid VideoFrame (e.g. a VideoFrame instance in
     * which releaseFrame() was called upon it will not call the callback).
     */
    VideoFrame(AVFrame* sourceFrame, QRect dimensions, int pixFmt, std::function<void()> destructCallback);
    VideoFrame(AVFrame* sourceFrame, std::function<void()> destructCallback);
    VideoFrame(AVFrame* sourceFrame);

    /**
     * Destructor for VideoFrame.
     */
    ~VideoFrame();

    // Copy/Move operations are disabled for the VideoFrame, encapsulate with a std::shared_ptr to manage.

    VideoFrame(const VideoFrame& other) = delete;
    VideoFrame(VideoFrame&& other) = delete;

    const VideoFrame& operator=(const VideoFrame& other) = delete;
    const VideoFrame& operator=(VideoFrame&& other) = delete;

    /**
     * @brief Returns the validity of this VideoFrame.
     *
     * A VideoFrame is valid if it manages at least one AVFrame. A VideoFrame can be invalidated
     * by calling releaseFrame() on it.
     *
     * @return true if the VideoFrame is valid, false otherwise.
     */
    bool isValid();

    /**
     * @brief Retrieves an AVFrame derived from the source based on the given parameters.
     *
     * If a given frame does not exist, this function will perform appropriate conversions to
     * return a frame that fulfills the given parameters.
     *
     * @param dimensions the dimensions of the frame.
     * @param pixelFormat the desired pixel format of the frame.
     * @return a pointer to a AVFrame with the given parameters or nullptr if the VideoFrame is no
     * longer valid.
     */
    const AVFrame* getAVFrame(const QSize& dimensions, const int pixelFormat);

    /**
     * @brief Releases all frames managed by this VideoFrame and invalidates it.
     */
    void releaseFrame();

    /**
     * @brief Retrieves a copy of the source VideoFrame's dimensions.
     * @return QRect copy representing the source VideoFrame's dimensions.
     */
    inline QRect getSourceDimensions()
    {
        return sourceDimensions;
    }

    /**
     * @brief Retrieves a copy of the source VideoFormat's pixel format.
     * @return integer copy represetning the source VideoFrame's pixel format.
     */
    inline int getSourcePixelFormat()
    {
        return sourcePixelFormat;
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
    QImage toQImage(QSize frameSize = {0, 0});

    /**
     * @brief Converts this VideoFrame to a vpx_image that shares this VideoFrame's buffer.
     *
     * Given that libvpx does not provide a way to create vpx_images that uses external buffers,
     * the vpx_image constructed by this function is done in a non-compliant way, requiring the
     * use of the C++ delete keyword to properly deallocate memory associated with this image.
     *
     * @param frameSize the given frame size of vpx_image to generate. If frame size is 0, defaults
     * to source frame size.
     * @return a vpx_image that represents this VideoFrame, sharing it's buffers or nullptr if this
     * VideoFrame is no longer valid.
     */
    vpx_image* toVpxImage(QSize frameSize = {0, 0});
private:
    /**
     * @brief A function to create a hashable key from a given QSize dimension.
     * @param dimensions the dimensions to hash.
     * @return a hashable unsigned 64-bit number representing the dimension.
     */
    static inline quint64 dimensionsToKey(const QSize& dimensions)
    {
        return (static_cast<quint64>(dimensions.width()) << 32) | dimensions.height();
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
     * @return a pointer to a AVFrame with the given parameters or nullptr if no such frame was
     * found.
     */
    AVFrame* retrieveAVFrame(const QSize& dimensions, const int pixelFormat);

    /**
     * @brief Generates an AVFrame based on the given specifications.
     *
     * This function is not thread-safe and must be called from a thread-safe context.
     *
     * @param dimensions the required dimensions for the frame.
     * @param pixelFormat the required pixel format for the frame.
     * @return an AVFrame with the given specifications.
     */
    AVFrame* generateAVFrame(const QSize& dimensions, const int pixelFormat);

    /**
     * @brief Stores a given AVFrame within the frameBuffer map.
     *
     * This function is not thread-safe and must be called from a thread-safe context.
     *
     * @param frame the given frame to store.
     * @param dimensions the dimensions of the frame.
     * @param pixelFormat the pixel format of the frame.
     */
    void storeAVFrame(AVFrame* frame, const QSize& dimensions, const int pixelFormat);

    /**
     * @brief Releases all frames within the frame buffer.
     *
     * This function is not thread-safe and must be called from a thread-safe context.
     */
    void deleteFrameBuffer();
private:
    // Data alignment for framebuffers
    static constexpr int data_alignment = 32;

    // Main framebuffer store
    QHash<int, QHash<quint64, AVFrame*>> frameBuffer {};

    // Source frame
    const QRect sourceDimensions;
    const int sourcePixelFormat;

    // Destructor callback
    const std::function<void ()> destructCallback;

    // Concurrency
    QReadWriteLock frameLock {};
};

#endif // VIDEOFRAME_H
