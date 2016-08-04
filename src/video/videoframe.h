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

class VideoFrame
{
public:
    explicit VideoFrame(AVFrame* frame);
    VideoFrame(AVFrame* frame, std::function<void()> freelistCallback);
    VideoFrame(AVFrame* frame, int w, int h, int fmt, std::function<void()> freelistCallback);
    ~VideoFrame();

    QSize getSize();

    void releaseFrame();

    QImage toQImage(QSize size = QSize());
    vpx_image* toVpxImage();

protected:
    bool convertToRGB24(QSize size = QSize());
    bool convertToYUV420();
    void releaseFrameLockless();

private:
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
