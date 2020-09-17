/*
    Copyright © 2015-2019 by The qTox Project Contributors

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


#pragma once

#include "videosource.h"
#include "core/icorevideo.h"

#include <QMutex>
#include <atomic>

class CoreVideoSource : public VideoSource, public ICoreVideo
{
    Q_OBJECT
public:
    // VideoSource interface
    void subscribe() override;
    void unsubscribe() override;

    CoreVideoSource();
    ~CoreVideoSource();

private:
    void stopSource();
    void restartSource();

private:
    std::atomic_int subscribers;
    QMutex biglock;
    std::atomic_bool stopped;

    // ICoreVideo interface
public:
    void pushFrame(const ToxStridedYUVFrame &frame) override;
};
