/*
    Copyright © 2019 by The qTox Project Contributors

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

#include <QMutex>
#include <QObject>

#include "util/interface.h"
#include "audio/iaudiosink.h"

class OpenAL;
class QMutex;
class AlSink : public QObject, public IAudioSink
{
    Q_OBJECT
public:
    AlSink(OpenAL& al, uint sourceId);
    AlSink(const AlSink& src) = delete;
    AlSink& operator=(const AlSink&) = delete;
    AlSink(AlSink&& other) = delete;
    AlSink& operator=(AlSink&& other) = delete;
    ~AlSink();

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels, int sampleRate) const override;
    void playMono16Sound(const IAudioSink::Sound& sound) override;
    void startLoop() override;
    void stopLoop() override;

    operator bool() const override;

    uint getSourceId() const;
    void kill();

    SIGNAL_IMPL(AlSink, finishedPlaying, void)
    SIGNAL_IMPL(AlSink, invalidated, void)

private:
    OpenAL& audio;
    uint sourceId;
    bool killed = false;
    mutable QMutex killLock;
};
