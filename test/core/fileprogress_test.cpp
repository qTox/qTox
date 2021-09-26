#include "src/core/toxfileprogress.h"

#include <QTest>
#include <limits>

class TestFileProgress : public QObject
{
    Q_OBJECT
private slots:
    void testSpeedLowSamples();
    void testSpeedUnevenIntervals();
    void testSpeedReset();
    void testSpeedRollingAverage();
    void testDiscardedSample();
    void testProgress();
    void testRemainingTime();
    void testBytesSentPersistence();
    void testFileSizePersistence();
    void testNoSamples();
    void testDefaultTimeLessThanNow();
    void testTimeChange();
    void testFinishedSpeed();
    void testSamplePeriod();
};

/**
 * @brief Test that when we're below the rolling average size our speeds are
 * sane
 */
void TestFileProgress::testSpeedLowSamples()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(0, nextSampleTime));
    // 1 sample has no speed
    QCOMPARE(progress.getSpeed(), 0.0);

    nextSampleTime = nextSampleTime.addMSecs(1000);
    // 10 bytes over 1s
    QVERIFY(progress.addSample(10, nextSampleTime));

    QCOMPARE(progress.getSpeed(), 10.0);

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(30, nextSampleTime));
    // 30 bytes over 2s
    QCOMPARE(progress.getSpeed(), 15.0);

}

/**
 * @brief Test that resetting our speed puts us back into a sane default state
 */
void TestFileProgress::testSpeedReset()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(0, nextSampleTime));

    // Push enough samples that we start rolling
    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(10, nextSampleTime));
    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(20, nextSampleTime));
    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(30, nextSampleTime));

    QCOMPARE(progress.getSpeed(), 10.0);

    progress.resetSpeed();
    QCOMPARE(progress.getSpeed(), 0.0);
    QCOMPARE(progress.lastSampleTime(), QTime());
    QCOMPARE(progress.getBytesSent(), uint64_t(30));
    QCOMPARE(progress.getProgress(), 0.3);

    // Ensure that pushing new samples after reset works correectly
    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(10, nextSampleTime));

    // 1 sample has no speed
    QCOMPARE(progress.getSpeed(), 0.0);

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(20, nextSampleTime));
    QCOMPARE(progress.getSpeed(), 10.0);
}


/**
 * @brief Test that when we pass over the rolling average boundary we still have
 * correct stats
 */
void TestFileProgress::testSpeedRollingAverage()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(0, nextSampleTime));

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(1, nextSampleTime));

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(2, nextSampleTime));

    // Rolling buffer maxed, we should see 0->2 over the span of 2 seconds
    QCOMPARE(progress.getSpeed(), 1.0);

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(4, nextSampleTime));

    // We should evict the first entry and see the average of 1->4 over the span
    // of 2 seconds
    QCOMPARE(progress.getSpeed(), 1.5);
}

/**
 * @brief Test that invalid samples are discarded
 */
void TestFileProgress::testDiscardedSample()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(0, nextSampleTime));

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(20, nextSampleTime));

    nextSampleTime = nextSampleTime.addMSecs(1000);

    // Sample should be discarded because it's too large
    QVERIFY(!progress.addSample(300, nextSampleTime));
    QCOMPARE(progress.lastSampleTime(), QTime(1, 0, 1));

    // Sample should be discarded because we're going backwards
    QVERIFY(!progress.addSample(10, nextSampleTime));
    QCOMPARE(progress.lastSampleTime(), QTime(1, 0, 1));
}

/**
 * @brief Test that progress is reported correctly
 */
void TestFileProgress::testProgress()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(0, nextSampleTime));
    QCOMPARE(progress.getProgress(), 0.0);

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(10, nextSampleTime));
    QCOMPARE(progress.getProgress(), 0.1);

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(100, nextSampleTime));
    QCOMPARE(progress.getProgress(), 1.0);
}

/**
 * @brief TEst that remaining time is predicted reasonably
 */
void TestFileProgress::testRemainingTime()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(0, nextSampleTime));

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(10, nextSampleTime));

    // 10% over 1s, 90% should take 9 more seconds
    QCOMPARE(progress.getTimeLeftSeconds(), 9.0);

    nextSampleTime = nextSampleTime.addMSecs(10000);
    QVERIFY(progress.addSample(100, nextSampleTime));
    // Even with a slow final sample, we should have 0 seconds remaining when we
    // are complete
    QCOMPARE(progress.getTimeLeftSeconds(), 0.0);
}

/**
 * @brief Test that the sent bytes keeps the last sample
 */
void TestFileProgress::testBytesSentPersistence()
{
    auto progress = ToxFileProgress(100, 2, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(10, nextSampleTime));

    // First sample
    QCOMPARE(progress.getBytesSent(), uint64_t(10));

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(20, nextSampleTime));
    // Second sample
    QCOMPARE(progress.getBytesSent(), uint64_t(20));

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(30, nextSampleTime));
    // After rollover
    QCOMPARE(progress.getBytesSent(), uint64_t(30));
}

/**
 * @brief Check that the reported file size matches what was given
 */
void TestFileProgress::testFileSizePersistence()
{
    auto progress = ToxFileProgress(33, 1000);
    QCOMPARE(progress.getFileSize(), uint64_t(33));
}

/**
 * @brief Test that we have sane stats when no samples have been added
 */
void TestFileProgress::testNoSamples()
{
    auto progress = ToxFileProgress(100, 1000);
    QCOMPARE(progress.getSpeed(), 0.0);
    QVERIFY(progress.getTimeLeftSeconds() == std::numeric_limits<double>::infinity());
    QCOMPARE(progress.getProgress(), 0.0);
}

/**
 * @brief Test that statistics are being average over the entire range of time
 * no matter the sample frequency
 */
void TestFileProgress::testSpeedUnevenIntervals()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(10, nextSampleTime));
    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(20, nextSampleTime));
    nextSampleTime = nextSampleTime.addMSecs(3000);
    QVERIFY(progress.addSample(50, nextSampleTime));

    // 10->50 over 4 seconds
    QCOMPARE(progress.getSpeed(), 10.0);


}

void TestFileProgress::testDefaultTimeLessThanNow()
{
    auto progress = ToxFileProgress(100, 3, 1000);
    QVERIFY(progress.lastSampleTime() < QTime::currentTime());
}

/**
 * @brief Test that changing the time resets the speed count. Note that it would
 * be better to use the monotonic clock, but it's not trivial to get the
 * monotonic clock from Qt's time API
 */
void TestFileProgress::testTimeChange()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(10, nextSampleTime));

    nextSampleTime = QTime(0, 0, 0);
    QVERIFY(progress.addSample(20, nextSampleTime));

    QCOMPARE(progress.getSpeed(), 0.0);
    QCOMPARE(progress.getProgress(), 0.2);

    nextSampleTime = QTime(0, 0, 1);
    QVERIFY(progress.addSample(30, nextSampleTime));
    QCOMPARE(progress.getSpeed(), 10.0);
}

/**
 * @brief Test that when a file is complete it's speed is set to 0
 */

void TestFileProgress::testFinishedSpeed()
{
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(10, nextSampleTime));

    nextSampleTime = nextSampleTime.addMSecs(1000);
    QVERIFY(progress.addSample(100, nextSampleTime));
    QCOMPARE(progress.getSpeed(), 0.0);
}

/**
 * @brief Test that we are averaged over the past period * samples time, and
 * when we roll we lose one sample period of data
 */
void TestFileProgress::testSamplePeriod()
{
    // No matter the number of samples, we should always be averaging over 2s
    auto progress = ToxFileProgress(100, 3, 1000);

    auto nextSampleTime = QTime(1, 0, 0);
    QVERIFY(progress.addSample(0, nextSampleTime));

    nextSampleTime = QTime(1, 0, 0, 500);
    QVERIFY(progress.addSample(10, nextSampleTime));

    // Even with less than a sample period our speed and size should be updated
    QCOMPARE(progress.getSpeed(), 20.0);
    QCOMPARE(progress.getBytesSent(), uint64_t(10));

    // Add a new sample at 1s, this should replace the previous sample
    nextSampleTime = QTime(1, 0, 1);
    QVERIFY(progress.addSample(30, nextSampleTime));
    QCOMPARE(progress.getSpeed(), 30.0);
    QCOMPARE(progress.getBytesSent(), uint64_t(30));

    // Add a new sample at 2s, our time should still be relative to 0
    nextSampleTime = QTime(1, 0, 2);
    QVERIFY(progress.addSample(50, nextSampleTime));
    // 50 - 0 over 2s
    QCOMPARE(progress.getSpeed(), 25.0);
    QCOMPARE(progress.getBytesSent(), uint64_t(50));

    nextSampleTime = QTime(1, 0, 1);
}

QTEST_GUILESS_MAIN(TestFileProgress)
#include "fileprogress_test.moc"
