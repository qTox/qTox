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

#include <QImage>
#include <QMutex>
#include <QReadWriteLock>
#include <QRect>
#include <QSize>

extern "C"{
#include <libavcodec/avcodec.h>
}

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

struct ToxYUVFrame
{
public:
    const std::uint16_t width;
    const std::uint16_t height;

    const uint8_t* y;
    const uint8_t* u;
    const uint8_t* v;
};

class VideoFrame
{
public:
    // Declare type aliases
    using IDType = std::uint_fast64_t;
    using AtomicIDType = std::atomic_uint_fast64_t;

public:
    VideoFrame(IDType sourceID, AVFrame* sourceFrame, QRect dimensions, int pixFmt, bool freeSourceFrame = false);
    VideoFrame(IDType sourceID, AVFrame* sourceFrame, bool freeSourceFrame = false);

    ~VideoFrame();

    // Copy/Move operations are disabled for the VideoFrame, encapsulate with a std::shared_ptr to manage.

    VideoFrame(const VideoFrame& other) = delete;
    VideoFrame(VideoFrame&& other) = delete;

    const VideoFrame& operator=(const VideoFrame& other) = delete;
    const VideoFrame& operator=(VideoFrame&& other) = delete;

    bool isValid();

    std::shared_ptr<VideoFrame> trackFrame();
    static void untrackFrames(const IDType& sourceID, bool releaseFrames = false);

    void releaseFrame();

    const AVFrame* getAVFrame(QSize frameSize, const int pixelFormat, const bool requireAligned);
    QImage toQImage(QSize frameSize = {});
    ToxYUVFrame toToxYUVFrame(QSize frameSize = {});

    /**
     * @brief Returns the ID for the given frame.
     *
     * Frame IDs are globally unique (with respect to the running instance).
     *
     * @return an integer representing the ID of this frame.
     */
    inline IDType getFrameID() const
    {
        return frameID;
    }

    /**
     * @brief Returns the ID for the VideoSource which created this frame.
     * @return an integer representing the ID of the VideoSource which created this frame.
     */
    inline IDType getSourceID() const
    {
        return sourceID;
    }

    /**
     * @brief Retrieves a copy of the source VideoFrame's dimensions.
     * @return QRect copy representing the source VideoFrame's dimensions.
     */
    inline QRect getSourceDimensions() const
    {
        return sourceDimensions;
    }

    /**
     * @brief Retrieves a copy of the source VideoFormat's pixel format.
     * @return integer copy represetning the source VideoFrame's pixel format.
     */
    inline int getSourcePixelFormat() const
    {
        return sourcePixelFormat;
    }

    static constexpr int dataAlignment = 32;

private:
    class FrameBufferKey{
    public:
        FrameBufferKey(const int width, const int height, const int pixFmt, const bool lineAligned);

        // Explictly state default constructor/destructor

        FrameBufferKey(const FrameBufferKey&) = default;
        FrameBufferKey(FrameBufferKey&&) = default;
        ~FrameBufferKey() = default;

        // Assignment operators are disabled for the FrameBufferKey

        const FrameBufferKey& operator=(const FrameBufferKey&) = delete;
        const FrameBufferKey& operator=(FrameBufferKey&&) = delete;

        /**
         * @brief Comparison operator for FrameBufferKey.
         *
         * @param other instance to compare against.
         * @return true if instances are equivilent, false otherwise.
         */
        inline bool operator==(const FrameBufferKey& other) const
        {
            return pixelFormat == other.pixelFormat &&
                   frameWidth == other.frameWidth &&
                   frameHeight == other.frameHeight &&
                   linesizeAligned == other.linesizeAligned;
        }

        /**
         * @brief Not equal to operator for FrameBufferKey.
         *
         * @param other instance to compare against
         * @return true if instances are not equivilent, false otherwise.
         */
        inline bool operator!=(const FrameBufferKey& other) const
        {
            return !operator==(other);
        }

        /**
         * @brief Hash function for class.
         *
         * This function computes a hash value for use with std::unordered_map.
         *
         * @param key the given instance to compute hash value of.
         * @return the hash of the given instance.
         */
        static inline size_t hash(const FrameBufferKey& key)
        {
            std::hash<int> intHasher;
            std::hash<bool> boolHasher;

            // Use java-style hash function to combine fields
            size_t ret = 47;

            ret = 37 * ret + intHasher(key.frameWidth);
            ret = 37 * ret + intHasher(key.frameHeight);
            ret = 37 * ret + intHasher(key.pixelFormat);
            ret = 37 * ret + boolHasher(key.linesizeAligned);

            return ret;
        }

    public:
        const int frameWidth;
        const int frameHeight;
        const int pixelFormat;
        const bool linesizeAligned;
    };

private:
    /**
     * @brief Generates a key object based on given parameters.
     *
     * @param frameSize the given size of the frame.
     * @param pixFmt the pixel format of the frame.
     * @param linesize the maximum linesize of the frame, may be larger than the width.
     * @return a FrameBufferKey object representing the key for the frameBuffer map.
     */
    static inline FrameBufferKey getFrameKey(const QSize& frameSize, const int pixFmt, const int linesize)
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
    static inline FrameBufferKey getFrameKey(const QSize& frameSize, const int pixFmt, const bool frameAligned)
    {
        return {frameSize.width(), frameSize.height(), pixFmt, frameAligned};
    }

    AVFrame* retrieveAVFrame(const QSize& dimensions, const int pixelFormat, const bool requireAligned);
    AVFrame* generateAVFrame(const QSize& dimensions, const int pixelFormat, const bool requireAligned);
    void storeAVFrame(AVFrame* frame, const QSize& dimensions, const int pixelFormat);

    void deleteFrameBuffer();

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
    T toGenericObject(const QSize& dimensions, const int pixelFormat, const bool requireAligned,
                      const std::function<T(AVFrame* const)> objectConstructor, const T& nullObject)
    {
        frameLock.lockForRead();

        // We return nullObject if the VideoFrame is no longer valid
        if(frameBuffer.size() == 0)
        {
            frameLock.unlock();
            return nullObject;
        }

        AVFrame* frame = retrieveAVFrame(dimensions, static_cast<int>(pixelFormat), requireAligned);

        if(frame)
        {
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

        storeAVFrame(frame, dimensions, static_cast<int>(pixelFormat));

        T ret = objectConstructor(frame);

        frameLock.unlock();
        return ret;
    }

private:
    // ID
    const IDType frameID;
    const IDType sourceID;

    // Main framebuffer store
    std::unordered_map<FrameBufferKey, AVFrame*, std::function<decltype(FrameBufferKey::hash)>> frameBuffer {3, FrameBufferKey::hash};

    // Source frame
    const QRect sourceDimensions;
    int sourcePixelFormat;
    const FrameBufferKey sourceFrameKey;
    const bool freeSourceFrame;

    // Reference store
    static AtomicIDType frameIDs;

    static std::unordered_map<IDType, QMutex> mutexMap;
    static std::unordered_map<IDType, std::unordered_map<IDType, std::weak_ptr<VideoFrame>>> refsMap;

    // Concurrency
    QReadWriteLock frameLock {};
    static QReadWriteLock refsLock;
};

#endif // VIDEOFRAME_H
