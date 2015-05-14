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

#include "videosurface.h"
#include "src/video/videoframe.h"
#include <QPainter>

VideoSurface::VideoSurface(QWidget* parent)
    : QWidget{parent}
    , source{nullptr}
    , frameLock{false}
    , hasSubscribed{false}
{
    
}

VideoSurface::VideoSurface(VideoSource *source, QWidget* parent)
    : VideoSurface(parent)
{
    setSource(source);
}

VideoSurface::~VideoSurface()
{
    unsubscribe();
}

void VideoSurface::setSource(VideoSource *src)
{
    if (source == src)
        return;

    unsubscribe();
    source = src;
    subscribe();
}

void VideoSurface::subscribe()
{
    if (source && !hasSubscribed)
    {
        source->subscribe();
        hasSubscribed = true;
        connect(source, &VideoSource::frameAvailable, this, &VideoSurface::onNewFrameAvailable);
    }
}

void VideoSurface::unsubscribe()
{
    if (!source || !hasSubscribed)
        return;

    // Fast lock
    {
        bool expected = false;
        while (!frameLock.compare_exchange_weak(expected, true))
            expected = false;
    }
    lastFrame.reset();
    frameLock = false;

    source->unsubscribe();
    hasSubscribed = false;
    disconnect(source, &VideoSource::frameAvailable, this, &VideoSurface::onNewFrameAvailable);
}

void VideoSurface::onNewFrameAvailable(std::shared_ptr<VideoFrame> newFrame)
{
    // Fast lock
    {
        bool expected = false;
        while (!frameLock.compare_exchange_weak(expected, true))
            expected = false;
    }

    lastFrame = newFrame;
    frameLock = false;
    update();
}

void VideoSurface::paintEvent(QPaintEvent*)
{
    // Fast lock
    {
        bool expected = false;
        while (!frameLock.compare_exchange_weak(expected, true))
            expected = false;
    }

    QPainter painter(this);
    painter.fillRect(painter.viewport(), Qt::black);
    if (lastFrame)
    {
        QRect rect = painter.viewport();
        QImage frame = lastFrame->toQImage();
        int width = frame.width()*rect.height()/frame.height();
        rect.setLeft((rect.width()-width)/2);
        rect.setWidth(width);
        painter.drawImage(rect, frame);
    }
    frameLock = false;
}
