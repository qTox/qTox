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
    VideoSurface(QWidget* parent=0);
    VideoSurface(VideoSource* source, QWidget* parent=0);
    ~VideoSurface();

    void setSource(VideoSource* src); //NULL is a valid option

protected:
    void subscribe();
    void unsubscribe();

    virtual void paintEvent(QPaintEvent * event) override;

private slots:
    void onNewFrameAvailable(std::shared_ptr<VideoFrame> newFrame);

private:
    VideoSource* source;
    std::shared_ptr<VideoFrame> lastFrame;
    std::atomic_bool frameLock; ///< Fast lock for lastFrame
    bool hasSubscribed;
};

#endif // SELFCAMVIEW_H
