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

#include "toxfileprogress.h"

#include <limits>

namespace
{
    size_t incrementRollingIdx(size_t idx, size_t max)
    {
        return (idx + 1) % max;
    }

    size_t decrementRollingIdx(size_t idx, size_t max)
    {
        if (idx == 0) {
            return max - 1;
        }

        return idx - 1;
    }
}

ToxFileProgress::ToxFileProgress(uint64_t filesize, size_t speedSampleCount, int samplePeriodMs)
    : filesize(filesize)
    , speedSampleCount(speedSampleCount)
    , samplePeriodMs(samplePeriodMs)
{
    if (samplePeriodMs < 0) {
        qWarning("Invalid sample rate, healing to 1000ms");
    }

    samples.reserve(speedSampleCount);
}

QTime ToxFileProgress::lastSampleTime() const
{
    if (samples.empty()) {
        return QTime();
    }

    return samples.back().timestamp;
}

bool ToxFileProgress::addSample(uint64_t bytesSent, QTime now)
{
    if (bytesSent > filesize) {
        qWarning("Bytes sent exceeds file size, ignoring sample");
        return false;
    }

    if (samples.size() > 0) {
        if (bytesSent < samples.back().bytesSent) {
            qWarning("Bytes sent has decreased since last sample, ignoring sample");
            return false;
        }

        if (now < samples.back().timestamp) {
            qWarning("Sample time has gone backwards, clearing progress buffer");
            resetSpeed();
        }
    }

    lastSample = {bytesSent, now};

    if (samples.size() > 1) {
        auto lastIdx = decrementRollingIdx(nextIdx, speedSampleCount);
        auto secondLastIdx = decrementRollingIdx(lastIdx, speedSampleCount);
        auto& last = samples[lastIdx];
        auto& secondLast = samples[secondLastIdx];
        // If we haven't hit a sample period yet we should replace the last
        // element with the latest update
        if (secondLast.timestamp.msecsTo(last.timestamp) < samplePeriodMs) {
            last = {bytesSent, now};
            return true;
        }
    }

    if (samples.size() < speedSampleCount) {
        samples.push_back({bytesSent, now});
        nextIdx = incrementRollingIdx(nextIdx, speedSampleCount);
    }
    else {
        samples[nextIdx] = {bytesSent, now};
        headIdx = incrementRollingIdx(headIdx, speedSampleCount);
        nextIdx = incrementRollingIdx(nextIdx, speedSampleCount);
    }

    return true;
}

void ToxFileProgress::resetSpeed()
{
    samples.clear();
    headIdx = 0;
    nextIdx = 0;
}

uint64_t ToxFileProgress::getBytesSent() const
{
    if (!lastSample) {
        return 0;
    }

    return lastSample.bytesSent;
}

double ToxFileProgress::getProgress() const
{
    if (!lastSample) {
        return 0;
    }

    return double(lastSample.bytesSent) / filesize;
}

double ToxFileProgress::getSpeed() const
{
    if (samples.size() > 0
        && samples[decrementRollingIdx(nextIdx, speedSampleCount)].bytesSent == filesize) {
        return 0;
    }

    if (samples.size() < 2) {
        return 0.0f;
    }

    const auto& tail = samples[decrementRollingIdx(nextIdx, speedSampleCount)];
    const auto& head = samples[headIdx];

    return (tail.bytesSent - head.bytesSent) / double(head.timestamp.msecsTo(tail.timestamp)) * 1000.0;
}

double ToxFileProgress::getTimeLeftSeconds() const
{
    if (samples.size() > 0
        && samples[decrementRollingIdx(nextIdx, speedSampleCount)].bytesSent == filesize) {
        return 0;
    }

    if (samples.size() < 2) {
        return std::numeric_limits<double>::infinity();
    }

    return double(filesize - samples[decrementRollingIdx(nextIdx, speedSampleCount)].bytesSent) / getSpeed();
}
