/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include <QObject>

#include <atomic>
#include <memory>

class VideoFrame;

class VideoSource : public QObject
{
    Q_OBJECT

public:
    // Declare type aliases
    using IDType = std::uint_fast64_t;
    using AtomicIDType = std::atomic_uint_fast64_t;

public:
    VideoSource()
        : id(sourceIDs++)
    {
    }

    virtual ~VideoSource() = default;
    /**
     * @brief If subscribe sucessfully opens the source, it will start emitting frameAvailable
     * signals.
     */
    virtual void subscribe() = 0;
    /**
     * @brief Stop emitting frameAvailable signals, and free associated resources if necessary.
     */
    virtual void unsubscribe() = 0;

    /// ID of this VideoSource
    const IDType id;
signals:
    /**
     * @brief Emitted when new frame available to use.
     * @param frame New frame.
     */
    void frameAvailable(std::shared_ptr<VideoFrame> frame);
    /**
     * @brief Emitted when the source is stopped for an indefinite amount of time, but might restart
     * sending frames again later
     */
    void sourceStopped();

private:
    // Used to manage a global ID for all VideoSources
    static AtomicIDType sourceIDs;
};
