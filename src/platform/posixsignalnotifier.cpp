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

#include "posixsignalnotifier.h"

#include <QDebug>
#include <QSocketNotifier>

#include <array>
#include <atomic>

#include <errno.h>
#include <signal.h>     // sigaction()
#include <sys/socket.h> // socketpair()
#include <sys/types.h>  // may be needed for BSD
#include <unistd.h>     // close()

/**
 * @class PosixSignalNotifier
 * @brief Class for converting POSIX signals to Qt signals
 */

namespace {

std::atomic_flag g_signalSocketUsageFlag = ATOMIC_FLAG_INIT;
std::array<int, 2> g_signalSocketPair;

void signalHandler(int signum)
{
    // DO NOT call any Qt functions directly, only limited amount of so-called async-signal-safe
    // functions can be called in signal handlers.
    // See https://doc.qt.io/qt-4.8/unix-signals.html

    // If test_and_set() returns true, it means it was already in use (only by ~PosixSignalNotifier()),
    // so we bail out. Our signal handler is blocking, only one will be called (no race between
    // threads), hence simple implementation.
    if (g_signalSocketUsageFlag.test_and_set())
        return;

    if(::write(g_signalSocketPair[0], &signum, sizeof(signum)) == -1) {
        // We hardly can do anything more usefull in signal handler, and
        // any ways it's probably very unexpected error (out of memory?),
        // since we check socket existance with a flag beforehand.
        abort();
    }

    g_signalSocketUsageFlag.clear();
}

} // namespace

PosixSignalNotifier::~PosixSignalNotifier()
{
    while (g_signalSocketUsageFlag.test_and_set()) {
        // spin-loop until we aquire flag (signal handler might be running and have flag in use)
    }

    // do not leak sockets
    ::close(g_signalSocketPair[0]);
    ::close(g_signalSocketPair[1]);

    // do not clear the usage flag here, signal handler cannot use socket any more!
}

void PosixSignalNotifier::watchSignal(int signum)
{
    sigset_t blockMask;
    sigemptyset(&blockMask); // do not prefix with ::, it's a macro on macOS
    sigaddset(&blockMask, signum); // do not prefix with ::, it's a macro on macOS

    struct sigaction action = {}; // all zeroes by default
    action.sa_handler = signalHandler;
    action.sa_mask = blockMask; // allow old signal to finish before new is raised

    if (::sigaction(signum, &action, nullptr)) {
        qFatal("Failed to setup signal %d, error = %d", signum, errno);
    }
}

void PosixSignalNotifier::watchSignals(std::initializer_list<int> signalSet)
{
    for (auto s: signalSet) {
        watchSignal(s);
    }
}

void PosixSignalNotifier::watchCommonTerminatingSignals()
{
    watchSignals({SIGHUP, SIGINT, SIGQUIT, SIGTERM});
}

PosixSignalNotifier& PosixSignalNotifier::globalInstance()
{
    static PosixSignalNotifier instance;
    return instance;
}

void PosixSignalNotifier::onSignalReceived()
{
    int signum{0};
    if (::read(g_signalSocketPair[1], &signum, sizeof(signum)) == -1) {
        qFatal("Failed to read from signal socket, error = %d", errno);
    }

    qDebug() << "Signal" << signum << "received";
    emit activated(signum);
}

PosixSignalNotifier::PosixSignalNotifier()
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, g_signalSocketPair.data())) {
       qFatal("Failed to create socket pair, error = %d", errno);
    }

    notifier = new QSocketNotifier(g_signalSocketPair[1], QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &PosixSignalNotifier::onSignalReceived);
}
