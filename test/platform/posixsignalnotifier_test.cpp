/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#include "src/platform/posixsignalnotifier.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include <signal.h>
#include <unistd.h>

class TestPosixSignalNotifier : public QObject
{
    Q_OBJECT
private slots:
    void checkUsrSignalHandling();
    void checkIgnoreExtraSignals();
    void checkTermSignalsHandling();
};

constexpr int TIMEOUT = 10;

void TestPosixSignalNotifier::checkUsrSignalHandling()
{
    PosixSignalNotifier& psn = PosixSignalNotifier::globalInstance();
    psn.watchSignal(SIGUSR1);
    QSignalSpy spy(&psn, &PosixSignalNotifier::activated);
    kill(getpid(), SIGUSR1);

    for (int i = 0; i < TIMEOUT && spy.count() != 1; ++i) {
        QCoreApplication::processEvents();
        sleep(1);
    }

    QCOMPARE(spy.count(), 1);
    const QList<QVariant>& args = spy.first();
    QCOMPARE(args.first().toInt(), SIGUSR1);
}

namespace {
void sighandler(int sig) {
    std::ignore = sig;
}
}

void TestPosixSignalNotifier::checkIgnoreExtraSignals()
{
    PosixSignalNotifier& psn = PosixSignalNotifier::globalInstance();
    psn.watchSignal(SIGUSR1);
    QSignalSpy spy(&psn, &PosixSignalNotifier::activated);

    // To avoid kiiling
    signal(SIGUSR2, sighandler);
    kill(getpid(), SIGUSR2);

    for (int i = 0; i < TIMEOUT; ++i) {
        QCoreApplication::processEvents();
        sleep(1);
    }

    QCOMPARE(spy.count(), 0);
}

void TestPosixSignalNotifier::checkTermSignalsHandling()
{
    PosixSignalNotifier& psn = PosixSignalNotifier::globalInstance();
    psn.watchCommonTerminatingSignals();
    QSignalSpy spy(&psn, &PosixSignalNotifier::activated);

    const std::initializer_list<int> termSignals = {SIGHUP, SIGINT, SIGQUIT, SIGTERM};
    for (int signal : termSignals) {
        QCoreApplication::processEvents();
        kill(getpid(), signal);
        sleep(1);
    }

    for (int i = 0; i < TIMEOUT && static_cast<size_t>(spy.size()) != termSignals.size(); i++) {
        QCoreApplication::processEvents();
        sleep(1);
    }

    QCOMPARE(static_cast<size_t>(spy.size()), termSignals.size());

    for (size_t i = 0; i < termSignals.size(); ++i) {
        const QList<QVariant>& args = spy.at(static_cast<int>(i));
        int signal = *(termSignals.begin() + i);
        QCOMPARE(args.first().toInt(), signal);
    }
}

QTEST_GUILESS_MAIN(TestPosixSignalNotifier)
#include "posixsignalnotifier_test.moc"
