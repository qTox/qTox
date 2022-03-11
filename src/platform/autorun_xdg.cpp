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

#include <QApplication>
#include "src/persistence/settings.h"
#include "src/platform/autorun.h"
#include <QDir>
#include <QProcessEnvironment>

namespace {
QString getAutostartDirPath()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString config = env.value("XDG_CONFIG_HOME");
    if (config.isEmpty())
        config = QDir::homePath() + "/" + ".config";
    return config + "/autostart";
}

QString getAutostartFilePath(const Settings& settings, QString dir)
{
    return dir + "/qTox - " + settings.getCurrentProfile() + ".desktop";
}

QString currentBinPath()
{
    const auto env = QProcessEnvironment::systemEnvironment();
    const auto appImageEnvKey = QStringLiteral("APPIMAGE");

    if (env.contains(appImageEnvKey)) {
        return env.value(appImageEnvKey);
    } else {
        return QApplication::applicationFilePath();
    }
}

inline QString profileRunCommand(const Settings& settings)
{
    return "\"" + currentBinPath() + "\" -p \""
           + settings.getCurrentProfile() + "\"";
}
} // namespace

bool Platform::setAutorun(const Settings& settings, bool on)
{
    QString dirPath = getAutostartDirPath();
    QFile desktop(getAutostartFilePath(settings, dirPath));
    if (on) {
        if (!QDir().mkpath(dirPath) || !desktop.open(QFile::WriteOnly | QFile::Truncate))
            return false;
        desktop.write("[Desktop Entry]\n");
        desktop.write("Type=Application\n");
        desktop.write("Name=qTox\n");
        desktop.write("Exec=");
        desktop.write(profileRunCommand(settings).toUtf8());
        desktop.write("\n");
        desktop.close();
        return true;
    } else
        return desktop.remove();
}

bool Platform::getAutorun(const Settings& settings)
{
    return QFile(getAutostartFilePath(settings, getAutostartDirPath())).exists();
}
