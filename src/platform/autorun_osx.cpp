/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "src/platform/autorun.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

static int state;

bool Platform::setAutorun(bool on)
{
    QString qtoxPlist =
        QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                        + QDir::separator() + "Library" + QDir::separator() + "LaunchAgents"
                        + QDir::separator() + "chat.tox.qtox.autorun.plist");
    QString qtoxDir =
        QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + "qtox");
    QSettings autoRun(qtoxPlist, QSettings::NativeFormat);
    autoRun.setValue("Label", "chat.tox.qtox.autorun");
    autoRun.setValue("Program", qtoxDir);

    state = on;
    autoRun.setValue("RunAtLoad", state);
    return true;
}

bool Platform::getAutorun()
{
    return state;
}
