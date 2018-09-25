#ifndef TOXFILEPROGRESS_H
#define TOXFILEPROGRESS_H

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


#endif // TOXFILEPROGRESS_H
