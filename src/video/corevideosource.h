/*
    Copyright © 2015 by The qTox Project Contributors

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

#include "videosource.h"
#include <QMutex>
#include <atomic>
#include <vpx/vpx_image.h>

class CoreVideoSource : public VideoSource
{
    Q_OBJECT
public:
    // VideoSource interface
    virtual void subscribe() override;
    virtual void unsubscribe() override;

private:
    CoreVideoSource();

    void pushFrame(const vpx_image_t* frame);
    void setDeleteOnClose(bool newstate);

    void stopSource();
    void restartSource();

private:
    std::atomic_int subscribers;
    std::atomic_bool deleteOnClose;
    QMutex biglock;
    std::atomic_bool stopped;

    friend class CoreAV;
    friend class ToxFriendCall;
};

#endif // COREVIDEOSOURCE_H
