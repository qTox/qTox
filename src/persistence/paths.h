/*
    Copyright © 2019 by The qTox Project Contributors

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

#include <QString>
#include <QStringList>

#define PATHS_VERSION_TCS_COMPLIANT 0

class Paths
{
public:
    enum class Portable {
        Auto,           /** Auto detect if portable or non-portable */
        Portable,       /** Force portable mode */
        NonPortable     /** Force non-portable mode */
    };

    static Paths* makePaths(Portable mode = Portable::Auto);

    bool isPortable() const;
#if PATHS_VERSION_TCS_COMPLIANT
    QString getGlobalSettingsPath() const;
    QString getProfilesDir() const;
    QString getToxSaveDir() const;
    QString getAvatarsDir() const;
    QString getTransfersDir() const;
    QStringList getThemeDirs() const;
    QString getScreenshotsDir() const;
#else
    // to be removed when paths migration is complete.
    QString getSettingsDirPath() const;
    QString getAppDataDirPath() const;
    QString getAppCacheDirPath() const;
    QString getUserNodesFilePath() const;
#endif


private:
    Paths(const QString &basePath, bool portable);

private:
    QString basePath{};
    const bool portable = false;
};
