/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include <QApplication>
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
#include "src/platform/autorun.h"
#include "src/persistence/settings.h"
#include <QProcessEnvironment>
#include <QDir>

namespace Platform
{
    QString getAutostartFilePath()
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString config = env.value("XDG_CONFIG_HOME");
        if (config.isEmpty())
            config = QDir::homePath() + "/" + ".config";
        return config + "/" + "autostart/qTox - " +
               Settings::getInstance().getCurrentProfile() + ".desktop";
    }

    inline QString currentCommandLine()
    {
        return "\"" + QApplication::applicationFilePath() + "\" -p \"" +
                Settings::getInstance().getCurrentProfile() + "\"";
    }
}

bool Platform::setAutorun(bool on)
{

    QFile desktop(getAutostartFilePath());
    if (on)
    {
        if (!desktop.open(QFile::WriteOnly | QFile::Truncate))
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
    return QFile(getAutostartFilePath()).exists();
}

#endif  // defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
