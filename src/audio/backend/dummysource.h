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

#ifndef DUMMYSOURCE_H
#define DUMMYSOURCE_H

#include "src/audio/iaudiosource.h"
#include <QMutex>
#include <QObject>
#include <atomic>

class OpenAL;
class DummySource : public IAudioSource
{
    Q_OBJECT
public:
    DummySource(OpenAL& al);
    DummySource(DummySource& src) = delete;
    DummySource& operator=(const DummySource&) = delete;
    DummySource(DummySource&& other) = delete;
    DummySource& operator=(DummySource&& other) = delete;
    ~DummySource();

    operator bool() const override;
    void kill() override;

private:
    OpenAL& audio;
    std::atomic<bool> killed{false};
};

#endif // DUMMYSOURCE_H
