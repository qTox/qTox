/*
    Copyright © 2015 by The qTox Project

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


#ifndef COREVIDEOSOURCE_H
#define COREVIDEOSOURCE_H

#include <vpx/vpx_image.h>
#include <atomic>
#include "videosource.h"
#include <QMutex>

/// A VideoSource that emits frames received by Core
class CoreVideoSource : public VideoSource
{
    Q_OBJECT
public:
    // VideoSource interface
    virtual bool subscribe() override;
    virtual void unsubscribe() override;

private:
    // Only CoreAV should create a CoreVideoSource since
    // only CoreAV can push images to it
    CoreVideoSource();

    /// Makes a copy of the vpx_image_t and emits it as a new VideoFrame
    void pushFrame(const vpx_image_t *frame);
    /// If true, self-delete after the last suscriber is gone
    void setDeleteOnClose(bool newstate);

    /// Stopping the source will block any pushFrame calls from doing anything
    /// See the callers in CoreAV for the rationale
    void stopSource();
    void restartSource();

private:
    std::atomic_int subscribers; ///< Number of suscribers
    std::atomic_bool deleteOnClose; ///< If true, self-delete after the last suscriber is gone
    QMutex biglock;
    std::atomic_bool stopped;

friend class CoreAV;
friend struct ToxFriendCall;
};

#endif // COREVIDEOSOURCE_H
