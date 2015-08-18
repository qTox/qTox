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
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/widget/friendwidget.h"
#include "src/persistence/settings.h"
#include <QPainter>
#include <QLabel>
#include <cassert>

float getSizeRatio(const QSize size)
{
    return size.width() / static_cast<float>(size.height());
}

VideoSurface::VideoSurface(int friendId, QWidget* parent, bool expanding)
    : QWidget{parent}
    , source{nullptr}
    , frameLock{false}
    , hasSubscribed{0}
    , friendId{friendId}
    , ratio{1.0f}
    , expanding{expanding}
{
    recalulateBounds();
}

VideoSurface::VideoSurface(int friendId, VideoSource *source, QWidget* parent)
    : VideoSurface(friendId, parent)
{
    setSource(source);
}

VideoSurface::~VideoSurface()
{
    unsubscribe();
}

bool VideoSurface::isExpanding() const
{
    return expanding;
}

void VideoSurface::setSource(VideoSource *src)
{
    if (source == src)
        return;

    unsubscribe();
    source = src;
    subscribe();
}

QRect VideoSurface::getBoundingRect() const
{
    QRect bRect = boundingRect;
    bRect.setBottomRight(QPoint(boundingRect.bottom() + 1, boundingRect.right() + 1));
    return boundingRect;
}

float VideoSurface::getRatio() const
{
    return ratio;
}

void VideoSurface::subscribe()
{
    assert(hasSubscribed >= 0);

    if (source && hasSubscribed++ == 0)
    {
        source->subscribe();
        connect(source, &VideoSource::frameAvailable, this, &VideoSurface::onNewFrameAvailable);
    }
}

void VideoSurface::unsubscribe()
{
    assert(hasSubscribed >= 0);

    if (!source || hasSubscribed == 0)
        return;

    if (--hasSubscribed != 0)
        return;

    lock();
    lastFrame.reset();
    unlock();

    ratio = 1.0f;
    recalulateBounds();
    emit ratioChanged();
    emit boundaryChanged();

    source->unsubscribe();
    disconnect(source, &VideoSource::frameAvailable, this, &VideoSurface::onNewFrameAvailable);
}

void VideoSurface::onNewFrameAvailable(std::shared_ptr<VideoFrame> newFrame)
{
    QSize newSize;

    lock();
    lastFrame = newFrame;
    newSize = lastFrame->getSize();
    unlock();

    float newRatio = getSizeRatio(newSize);

    if (newRatio != ratio && isVisible())
    {
        ratio = newRatio;
        recalulateBounds();
        emit ratioChanged();
        emit boundaryChanged();
    }

    update();
}
#include "src/core/core.h"
#include "src/widget/style.h"
#include <QDebug>
void VideoSurface::paintEvent(QPaintEvent*)
{
    lock();

    QPainter painter(this);
    painter.fillRect(painter.viewport(), Qt::black);
    if (lastFrame)
    {
        QImage frame = lastFrame->toQImage(rect().size());
        painter.drawImage(boundingRect, frame, frame.rect(), Qt::NoFormatConversion);
    }
    else
    {
        painter.fillRect(boundingRect, Qt::white);
        QPixmap avatar;

        QString userId;

        if (friendId != -1)
            userId = FriendList::findFriend(friendId)->getToxId().toString();
        else
            userId = Core::getInstance()->getSelfId().toString();

        avatar = Settings::getInstance().getSavedAvatar(userId);

        if (avatar.isNull())
            avatar = Style::scaleSvgImage(":/img/contact_dark.svg", boundingRect.width(), boundingRect.height());

        painter.drawPixmap(boundingRect, avatar, avatar.rect());
    }

    unlock();
}

void VideoSurface::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    recalulateBounds();
    emit boundaryChanged();
}

void VideoSurface::showEvent(QShowEvent*)
{
    //emit ratioChanged();
}
#include <QDebug>
void VideoSurface::recalulateBounds()
{
    if (expanding)
    {
        boundingRect = contentsRect();
    }
    else
    {
        QPoint pos;
        QSize size;
        QSize usableSize = contentsRect().size();
        int possibleWidth = usableSize.height() * ratio;

        if (possibleWidth > usableSize.width())
            size = (QSize(usableSize.width(), usableSize.width() / ratio));
        else
            size = (QSize(possibleWidth, usableSize.height()));

        pos.setX(width() / 2 - size.width() / 2);
        pos.setY(height() / 2 - size.height() / 2);
        boundingRect.setRect(pos.x(), pos.y(), size.width(), size.height());
    }

    qDebug() << contentsRect();

    update();
}

void VideoSurface::lock()
{
    // Fast lock
    bool expected = false;
    while (!frameLock.compare_exchange_weak(expected, true))
        expected = false;
}

void VideoSurface::unlock()
{
    frameLock = false;
}
