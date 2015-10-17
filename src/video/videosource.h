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

#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QObject>
#include <memory>

class VideoFrame;

/// An abstract source of video frames
/// When it has at least one subscriber the source will emit new video frames
/// Subscribing is recursive, multiple users can subscribe to the same VideoSource
class VideoSource : public QObject
{
    Q_OBJECT

public:
    virtual ~VideoSource() = default;
    /// If subscribe sucessfully opens the source, it will start emitting frameAvailable signals
    virtual bool subscribe() = 0;
    /// Stop emitting frameAvailable signals, and free associated resources if necessary
    virtual void unsubscribe() = 0;

signals:
    void frameAvailable(std::shared_ptr<VideoFrame> frame);
};

#endif // VIDEOSOURCE_H
