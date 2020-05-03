/*
    Copyright © 2018-2019 by The qTox Project Contributors

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

#include <QTime>

struct ToxFile;

class ToxFileProgress
{
public:
    bool needsUpdate() const;
    void addSample(ToxFile const& file);
    void resetSpeed();

    double getProgress() const;
    double getSpeed() const;
    double getTimeLeftSeconds() const;

private:
    uint64_t lastBytesSent = 0;

    static const uint8_t TRANSFER_ROLLING_AVG_COUNT = 4;
    uint8_t meanIndex = 0;
    double meanData[TRANSFER_ROLLING_AVG_COUNT] = {0.0};

    QTime lastTick = QTime::currentTime();

    double speedBytesPerSecond;
    double timeLeftSeconds;
    double progress;
};
