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

#include "videosurface.h"
#include "src/video/videoframe.h"
#include <QPainter>
#include <QLabel>

VideoSurface::VideoSurface(QWidget* parent)
    : QWidget{parent}
    , source{nullptr}
    , frameLock{false}
    , hasSubscribed{false}
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
#include <QDebug>
QRect VideoSurface::getRect() const
{
    // Fast lock
    {
        bool expected = false;
        while (!frameLock.compare_exchange_weak(expected, true))
            expected = false;
    }

    std::shared_ptr<VideoFrame> last = lastFrame;
    frameLock = false;

    if (last)
    {
        QSize frameSize = lastFrame->getSize();
        QRect rect = this->rect();
        int width = frameSize.width()*rect.height()/frameSize.height();
        rect.setLeft((rect.width()-width)/2);
        rect.setWidth(width);
        return rect;
    }

    return QRect();
}

QSize VideoSurface::getFrameSize()
{
    // Fast lock
    {
        bool expected = false;
        while (!frameLock.compare_exchange_weak(expected, true))
            expected = false;
    }

    QSize frameSize;

    if (lastFrame)
        frameSize = lastFrame->getSize();

    frameLock = false;
    return frameSize;
}

QSize VideoSurface::sizeHint() const
{
    return getRect().size();
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

    emit drewNewFrame();
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
    //painter.fillRect(painter.viewport(), Qt::black);
    if (lastFrame)
    {
        QSize frameSize = lastFrame->getSize();
        QRect rect = this->rect();
        int width = frameSize.width()*rect.height()/frameSize.height();
        rect.setLeft((rect.width()-width)/2);
        rect.setWidth(width);

        QImage frame = lastFrame->toQImage(rect.size());
        painter.drawImage(rect, frame, frame.rect(), Qt::NoFormatConversion);
        //qDebug() << "VIDEO 2" << rect;
    }
    frameLock = false;
}
#include <QResizeEvent>
void VideoSurface::resizeEvent(QResizeEvent* event)
{
    // Locks aspect ratio.
    QSize frameSize;

    // Fast lock
    {
        bool expected = false;
        while (!frameLock.compare_exchange_weak(expected, true))
            expected = false;
    }

    if (lastFrame)
    {
        frameSize = lastFrame->getSize();
    }

    frameLock = false;

    if (frameSize.isValid())
    {
        float ratio = frameSize.height() / static_cast<float>(frameSize.width());
        int width = ratio*event->size().width();
        setMaximumHeight(width);
    }
}
