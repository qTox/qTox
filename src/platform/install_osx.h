/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include <QtCore/qsystemdetection.h>

#ifndef Q_OS_OSX
#error "This file is only meant to be compiled for Mac OSX targets"
#endif

namespace osx {
static constexpr int EXIT_UPDATE_MACX =
    218; // We track our state using unique exit codes when debugging
static constexpr int EXIT_UPDATE_MACX_FAIL = 216;

void moveToAppFolder();
void migrateProfiles();
}
