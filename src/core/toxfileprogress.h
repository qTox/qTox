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

#include <array>

class ToxFileProgress
{
public:
    ToxFileProgress(uint64_t filesize_, int samplePeriodMs_ = 4000);

    QTime lastSampleTime() const;
    bool addSample(uint64_t bytesSent, QTime now = QTime::currentTime());
    void resetSpeed();

    uint64_t getBytesSent() const;
    uint64_t getFileSize() const { return filesize; }
    double getProgress() const;
    double getSpeed() const;
    double getTimeLeftSeconds() const;

private:
    // Should never be modified, but do not want to lose assignment operators
    uint64_t filesize;
    int samplePeriodMs;

    struct Sample
    {
        uint64_t bytesSent = 0;
        QTime timestamp;
    };

    std::array<Sample, 2> samples;
    uint8_t activeSample = 0;
};
