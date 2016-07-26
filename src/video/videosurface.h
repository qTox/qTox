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

#ifndef SELFCAMVIEW_H
#define SELFCAMVIEW_H

#include <QWidget>
#include <memory>
#include <atomic>
#include "src/video/videosource.h"

class VideoSurface : public QWidget
{
    Q_OBJECT

public:
    VideoSurface(const QPixmap& avatar, QWidget* parent = 0, bool expanding = false);
    VideoSurface(const QPixmap& avatar, VideoSource* source, QWidget* parent = 0);
    ~VideoSurface();

    bool isExpanding() const;
    void setSource(VideoSource* src);
    QRect getBoundingRect() const;
    float getRatio() const;
    void setAvatar(const QPixmap& pixmap);
    QPixmap getAvatar() const;

signals:
    void ratioChanged();
    void boundaryChanged();

protected:
    void subscribe();
    void unsubscribe();

    virtual void paintEvent(QPaintEvent* event) final override;
    virtual void resizeEvent(QResizeEvent* event) final override;
    virtual void showEvent(QShowEvent* event) final override;

private slots:
    void onNewFrameAvailable(std::shared_ptr<VideoFrame> newFrame);
    void onSourceStopped();

private:
    void recalulateBounds();
    void lock();
    void unlock();

    QRect boundingRect;
    VideoSource* source;
    std::shared_ptr<VideoFrame> lastFrame;
    std::atomic_bool frameLock;
    uint8_t hasSubscribed;
    QPixmap avatar;
    float ratio;
    bool expanding;
};

#endif // SELFCAMVIEW_H
