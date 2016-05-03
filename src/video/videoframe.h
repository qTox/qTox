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

#include <QMutex>
#include <QImage>
#include <functional>

struct AVFrame;
struct AVCodecContext;
struct vpx_image;

/// VideoFrame takes ownership of an AVFrame* and allows fast conversions to other formats
/// Ownership of all video frame buffers is kept by the VideoFrame, even after conversion
/// All references to the frame data become invalid when the VideoFrame is deleted
/// We try to avoid pixel format conversions as much as possible, at the cost of some memory
/// All methods are thread-safe. If provided freelistCallback will be called by the destructor,
/// unless releaseFrame was called in between.
class VideoFrame
{
public:
    explicit VideoFrame(AVFrame* frame);
    VideoFrame(AVFrame* frame, std::function<void()> freelistCallback);
    VideoFrame(AVFrame* frame, int w, int h, int fmt, std::function<void()> freelistCallback);
    ~VideoFrame();

    /// Return the size of the original frame
    QSize getSize();

    /// Frees all internal buffers and frame data, removes the freelistCallback
    /// This makes all converted objects that shares our internal buffers invalid
    void releaseFrame();

    /// Converts the VideoFrame to a QImage that shares our internal video buffer
    QImage toQImage(QSize size = QSize());
    /// Converts the VideoFrame to a vpx_image_t that shares our internal video buffer
    /// Free it with operator delete, NOT vpx_img_free
    vpx_image* toVpxImage();

protected:
    bool convertToRGB24(QSize size = QSize());
    bool convertToYUV420();
    void releaseFrameLockless();

private:
    // Disable copy. Use a shared_ptr if you need copies.
    VideoFrame(const VideoFrame& other)=delete;
    VideoFrame& operator=(const VideoFrame& other)=delete;

private:
    std::function<void()> freelistCallback;
    QMutex biglock;
    AVFrame* frameOther, *frameYUV420, *frameRGB24;
    int width, height;
    int pixFmt;
};

#endif // VIDEOFRAME_H
