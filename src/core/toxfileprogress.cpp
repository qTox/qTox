/*
    Copyright © 2021 by The qTox Project Contributors

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

#include "toxfileprogress.h"

#include <limits>

ToxFileProgress::ToxFileProgress(uint64_t filesize_, int samplePeriodMs_)
    : filesize(filesize_)
    , samplePeriodMs(samplePeriodMs_)
{
    if (samplePeriodMs_ < 0) {
        qWarning("Invalid sample rate, healing to 1000ms");
        samplePeriodMs = 1000;
    }
}

QTime ToxFileProgress::lastSampleTime() const
{
    return samples[activeSample].timestamp;
}

bool ToxFileProgress::addSample(uint64_t bytesSent, QTime now)
{
    if (bytesSent > filesize) {
        qWarning("Bytes sent exceeds file size, ignoring sample");
        return false;
    }

    auto* active = &samples[activeSample];
    auto* inactive = &samples[!activeSample];

    if (bytesSent < active->bytesSent || bytesSent < inactive->bytesSent) {
        qWarning("Bytes sent has decreased since last sample, ignoring sample");
        return false;
    }

    if (now < active->timestamp || now < inactive->timestamp) {
        qWarning("Sample time has gone backwards, clearing progress buffer");
        resetSpeed();
    }

    // Ensure both samples are initialized
    if (inactive->timestamp == QTime()) {
        inactive->bytesSent = bytesSent;
        inactive->timestamp = now;
    }

    if (active->timestamp == QTime()) {
        active->bytesSent = bytesSent;
        active->timestamp = now;
    }

    if (active->timestamp.msecsTo(now) >= samplePeriodMs) {
        // Swap samples and set the newly active sample
        activeSample = !activeSample;
        std::swap(active, inactive);
    }

    active->bytesSent = bytesSent;
    active->timestamp = now;

    return true;
}

void ToxFileProgress::resetSpeed()
{
    for (auto& sample : samples) {
        sample.timestamp = QTime();
    }
}

uint64_t ToxFileProgress::getBytesSent() const
{
    return samples[activeSample].bytesSent;
}

double ToxFileProgress::getProgress() const
{
    return double(samples[activeSample].bytesSent) / filesize;
}

double ToxFileProgress::getSpeed() const
{
    if (samples.size() > 0
        && samples[activeSample].bytesSent == filesize) {
        return 0.0;
    }

    const auto sampleTimeInvalid = [](const Sample& sample) {
        return sample.timestamp == QTime();
    };

    if (std::any_of(samples.cbegin(), samples.cend(), sampleTimeInvalid)) {
        return 0.0;
    }

    if (samples[0].timestamp == samples[1].timestamp) {
        return 0.0;
    }

    const auto& active = samples[activeSample];
    const auto& inactive = samples[!activeSample];

    return (active.bytesSent - inactive.bytesSent) / double(inactive.timestamp.msecsTo(active.timestamp)) * 1000.0;
}

double ToxFileProgress::getTimeLeftSeconds() const
{
    if (samples.size() > 0
        && samples[activeSample].bytesSent == filesize) {
        return 0;
    }

    const auto speed = getSpeed();
    if (speed == 0.0) {
        return std::numeric_limits<double>::infinity();
    }

    return double(filesize - samples[activeSample].bytesSent) / getSpeed();
}
