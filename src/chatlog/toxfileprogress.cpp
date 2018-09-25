#include "toxfileprogress.h"

#include "src/core/toxfile.h"

bool ToxFileProgress::needsUpdate() const
{
    QTime now = QTime::currentTime();
    qint64 dt = lastTick.msecsTo(now); // ms

    if (dt < 1000)
        return false;
    return true;
}

void ToxFileProgress::addSample(ToxFile const& file)
{
    QTime now = QTime::currentTime();
    qint64 dt = lastTick.msecsTo(now); // ms

    if (dt < 1000)
        return;

    // ETA, speed
    qreal deltaSecs = dt / 1000.0;

    // (can't use ::abs or ::max on unsigned types substraction, they'd just overflow)
    quint64 deltaBytes = file.bytesSent > lastBytesSent ? file.bytesSent - lastBytesSent
                                                        : lastBytesSent - file.bytesSent;
    qreal bytesPerSec = static_cast<int>(static_cast<qreal>(deltaBytes) / deltaSecs);

    // Update member variables
    meanIndex = meanIndex % TRANSFER_ROLLING_AVG_COUNT;
    meanData[meanIndex++] = bytesPerSec;

    double meanBytesPerSec = 0.0;
    for (size_t i = 0; i < TRANSFER_ROLLING_AVG_COUNT; ++i)
        meanBytesPerSec += meanData[i];
    meanBytesPerSec /= static_cast<qreal>(TRANSFER_ROLLING_AVG_COUNT);

    lastTick = now;

    progress = static_cast<double>(file.bytesSent) / static_cast<double>(file.filesize);
    speedBytesPerSecond = meanBytesPerSec;
    timeLeftSeconds = (file.filesize - file.bytesSent) / getSpeed();

    lastBytesSent = file.bytesSent;
}

void ToxFileProgress::resetSpeed()
{
    meanIndex = 0;
    for (auto& item : meanData) {
        item = 0;
    }
}

double ToxFileProgress::getProgress() const
{
    return progress;
}

double ToxFileProgress::getSpeed() const
{
    return speedBytesPerSecond;
}

double ToxFileProgress::getTimeLeftSeconds() const
{
    return timeLeftSeconds;
}
