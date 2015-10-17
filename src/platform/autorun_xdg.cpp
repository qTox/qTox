/*
    Copyright © 2014-2015 by The qTox Project

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

#include <QApplication>
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
#include "src/platform/autorun.h"
#include "src/persistence/settings.h"
#include <QProcessEnvironment>
#include <QDir>

namespace Platform
{
    QString getAutostartDirPath()
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString config = env.value("XDG_CONFIG_HOME");
        if (config.isEmpty())
            config = QDir::homePath() + "/" + ".config";
        return config + "/autostart";
    }

    QString getAutostartFilePath(QString dir)
    {
        return dir + "/qTox - " + Settings::getInstance().getCurrentProfile() +
               ".desktop";
    }

    inline QString currentCommandLine()
    {
        return "\"" + QApplication::applicationFilePath() + "\" -p \"" +
                Settings::getInstance().getCurrentProfile() + "\"";
    }
}

bool Platform::setAutorun(bool on)
{
    QString dirPath = getAutostartDirPath();
    QFile desktop(getAutostartFilePath(dirPath));
    if (on)
    {
        if (!QDir().mkpath(dirPath) ||
            !desktop.open(QFile::WriteOnly | QFile::Truncate))
            return false;
        desktop.write("[Desktop Entry]\n");
        desktop.write("Type=Application\n");
        desktop.write("Name=qTox\n");
        desktop.write("Exec=");
        desktop.write(currentCommandLine().toUtf8());
        desktop.write("\n");
        desktop.close();
        return true;
    }
    else
        return desktop.remove();
}

bool Platform::getAutorun()
{
    return QFile(getAutostartFilePath(getAutostartDirPath())).exists();
}

#endif  // defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
