/*
    Copyright © 2017 by The qTox Project Contributors

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

#include <errno.h>
#include <signal.h>     // sigaction()
#include <sys/socket.h> // socketpair()
#include <sys/types.h>  // may be needed for BSD
#include <unistd.h>     // close()

/**
 * @class PosixSignalNotifier
 * @brief Class for converting POSIX signals to Qt signals
 */

std::array<int, 2> PosixSignalNotifier::socketPair;

PosixSignalNotifier::~PosixSignalNotifier()
{
    // to avoid race condition, swap socket descriptors
    // so that signal handler would see zeroes before sockets
    // are being freed.
    std::array<int, 2> pair;
    std::swap(pair, socketPair); // socketPair is now zeroed

    // do not leak sockets
    ::close(pair[0]);
    ::close(pair[1]);
}

void PosixSignalNotifier::watchSignal(int signum)
{
    sigset_t blockMask;
    ::sigemptyset(&blockMask);
    ::sigaddset(&blockMask, signum);

    struct sigaction action = {}; // all zeroes by default
    action.sa_handler = PosixSignalNotifier::signalHandler;
    action.sa_mask = blockMask; // allow old signal to finish before new is raised

    if (::sigaction(signum, &action, 0)) {
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

PosixSignalNotifier* PosixSignalNotifier::globalInstance()
{
    static PosixSignalNotifier instance;
    return &instance;
}

void PosixSignalNotifier::onSignalReceived()
{
    int signum = 0;
    ::read(socketPair[1], &signum, sizeof(signum));

    qDebug() << "Signal" << signum << "received";
    emit activated(signum);
}

PosixSignalNotifier::PosixSignalNotifier()
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, socketPair.data())) {
       qFatal("Failed to create socket pair, error = %d", errno);
    }

    notifier = new QSocketNotifier(socketPair[1], QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &PosixSignalNotifier::onSignalReceived);
}

void PosixSignalNotifier::signalHandler(int signum)
{
    // DO NOT call any Qt functions directly, only limited amount of so-called async-signal-safe
    // functions can be called in signal handlers.
    // See https://doc.qt.io/qt-4.8/unix-signals.html

    if (!socketPair[0]) {
        return; // do nothing if socket are already closed (app is quiting already, see ~PosixSignalNotifier())
    }

    ::write(socketPair[0], &signum, sizeof(signum));
}
