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

#include "paths.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>
#include <QStringBuilder>
#include <QStringList>
#include <QSettings>

#include <cassert>

namespace {
const QLatin1String globalSettingsFile{"qtox.ini"};

#if PATHS_VERSION_TCS_COMPLIANT
const QLatin1String profileFolder{"profiles"};
const QLatin1String themeFolder{"themes"};
const QLatin1String avatarsFolder{"avatars"};
const QLatin1String transfersFolder{"transfers"};
const QLatin1String screenshotsFolder{"screenshots"};

// NOTE(sudden6): currently unused, but reflects the TCS at 2018-11
#ifdef Q_OS_WIN
const QLatin1String TCSToxFileFolder{"%APPDATA%/tox/"};
#elif defined(Q_OS_OSX)
const QLatin1String TCSToxFileFolder{"~/Library/Application Support/Tox"};
#else
const QLatin1String TCSToxFileFolder{"~/.config/tox/"};
#endif
#endif // PATHS_VERSION_TCS_COMPLIANT
} // namespace

/**
 * @class Profile
 * @brief Handles all qTox internal paths
 *
 * The qTox internal file layout starts at `<BASE_PATH>`. This directory is platform
 * specific and depends on if qTox runs in portable mode.
 *
 * Example file layout for non-portable mode:
 * @code
 *  <BASE_PATH>/themes/
 *             /profiles/
 *             /profiles/avatars/
 *             /...
 * @endcode
 *
 * Example file layout for portable mode:
 * @code
 *  /qTox.bin
 *  /themes/
 *  /profiles/
 *  /profiles/avatars/
 *  /qtox.ini
 * @endcode
 *
 * All qTox or Tox specific directories should be looked up through this module.
 */

namespace {
    using Portable = Paths::Portable;
    bool portableFromMode(Portable mode)
    {
        bool portable = false;
        const QString basePortable = qApp->applicationDirPath();
        const QString baseNonPortable = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QString portableSettingsPath = basePortable % QDir::separator() % globalSettingsFile;

        switch (mode) {
        case Portable::Portable:
            qDebug() << "Forcing portable";
            return true;
        case Portable::NonPortable:
            qDebug() << "Forcing non-portable";
            return false;
        case Portable::Auto:
            if (QFile(portableSettingsPath).exists()) {
                QSettings ps(portableSettingsPath, QSettings::IniFormat);
                ps.setIniCodec("UTF-8");
                ps.beginGroup("Advanced");
                portable = ps.value("makeToxPortable", false).toBool();
                ps.endGroup();
            } else {
                portable = false;
            }
            qDebug()<< "Auto portable mode:" << portable;
            return portable;
        }
        assert(!"Unhandled enum class Paths::Portable value");
        return false;
    }

    QString basePathFromPortable(bool portable)
    {
        const QString basePortable = qApp->applicationDirPath();
        const QString baseNonPortable = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString basePath = portable ? basePortable : baseNonPortable;
        if (basePath.isEmpty()) {
            assert(!"Couldn't find writeable path");
            qCritical() << "Couldn't find writable path";
        }
        return basePath;
    }
} // namespace

Paths::Paths(Portable mode)
{
    portable = portableFromMode(mode);
    basePath = basePathFromPortable(portable);
}

/**
 * @brief Set paths to passed portable or system wide
 * @param newPortable
 * @return True if portable mode changed, else false.
 */
bool Paths::setPortable(bool newPortable)
{
    auto oldVal = portable.exchange(newPortable);
    if (oldVal != newPortable) {
        return true;
    }
    return false;
}

/**
 * @brief Check if qTox is running in portable mode.
 * @return True if running in portable mode, false else.
 */
bool Paths::isPortable() const
{
    return portable;
}

#if PATHS_VERSION_TCS_COMPLIANT
/**
 * @brief Returns the path to the global settings file "qtox.ini"
 * @return The path to the folder.
 */
QString Paths::getGlobalSettingsPath() const
{
    QString path;

    if (portable) {
        path = basePath;
    } else {
        path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        if (path.isEmpty()) {
            qDebug() << "Can't find writable location for settings file";
            return {};
        }
    }

    // we assume a writeable path for portable mode

    return path % QDir::separator() % globalSettingsFile;
}

/**
 * @brief Get the folder where profile specific information is stored, e.g. <profile>.ini
 * @return The path to the folder.
 */
QString Paths::getProfilesDir() const
{
    return basePath % QDir::separator() % profileFolder % QDir::separator();
}

/**
 * @brief Get the folder where the <profile>.tox file is stored
 * @note Expect a change here, since TCS will probably be updated.
 * @return The path to the folder on success, empty string else.
 */
QString Paths::getToxSaveDir() const
{
    if (isPortable()) {
        return basePath % QDir::separator() % profileFolder % QDir::separator();
    }

        // GenericDataLocation would be a better solution, but we keep this code for backward
        // compatibility

// workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    // TODO(sudden6): this doesn't really follow the Tox Client Standard and probably
    // breaks when %APPDATA% is changed
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "AppData" + QDir::separator() + "Roaming"
                           + QDir::separator() + "tox")
           + QDir::separator();
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "Library" + QDir::separator()
                           + "Application Support" + QDir::separator() + "Tox")
           + QDir::separator();
#else
    // TODO(sudden6): This does not respect the XDG_* environment variables and also
    // stores user data in a config location
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                           + QDir::separator() + "tox")
           + QDir::separator();
#endif
}

/**
 * @brief Get the folder where avatar files are stored
 * @note Expect a change here, since TCS will probably be updated.
 * @return The path to the folder on success, empty string else.
 */
QString Paths::getAvatarsDir() const
{
    // follow the layout in
    // https://tox.gitbooks.io/tox-client-standard/content/data_storage/export_format.html
    QString path = getToxSaveDir();
    if (path.isEmpty()) {
        qDebug() << "Can't find location for avatars directory";
        return {};
    }

    return path % avatarsFolder % QDir::separator();
}

/**
 * @brief Get the folder where screenshots are stored
 * @return The path to the folder.
 */
QString Paths::getScreenshotsDir() const
{
    return basePath % QDir::separator() % screenshotsFolder % QDir::separator();
}

/**
 * @brief Get the folder where file transfer data is stored
 * @return The path to the folder.
 */
QString Paths::getTransfersDir() const
{
    return basePath % QDir::separator() % transfersFolder % QDir::separator();
}

/**
 * @brief Get a prioritized list with directories that contain themes.
 * @return A list of directories sorted from most important to least important.
 * @note Users of this function should use the theme from the folder that appears first in the list.
 */
QStringList Paths::getThemeDirs() const
{
    QStringList themeFolders{};

    if (!isPortable()) {
        themeFolders += QStandardPaths::locate(QStandardPaths::AppDataLocation, themeFolder,
                                               QStandardPaths::LocateDirectory);
    }

    // look for themes beside the qTox binary with lowest priority
    const QString curPath = qApp->applicationDirPath();
    themeFolders += curPath % QDir::separator() % themeFolder % QDir::separator();

    return themeFolders;
}

#else
/**
 * @brief Get path to directory, where the settings files are stored.
 * @return Path to settings directory, ends with a directory separator.
 * @note To be removed when path migration is complete.
 */
QString Paths::getSettingsDirPath() const
{
    if (portable)
        return qApp->applicationDirPath() + QDir::separator();

// workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "AppData" + QDir::separator() + "Roaming"
                           + QDir::separator() + "tox")
           + QDir::separator();
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "Library" + QDir::separator()
                           + "Application Support" + QDir::separator() + "Tox")
           + QDir::separator();
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                           + QDir::separator() + "tox")
           + QDir::separator();
#endif
}

/**
 * @brief Get path to directory, where the application data are stored.
 * @return Path to application data, ends with a directory separator.
 * @note To be removed when path migration is complete.
 */
QString Paths::getAppDataDirPath() const
{
    if (portable)
        return qApp->applicationDirPath() + QDir::separator();

// workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "AppData" + QDir::separator() + "Roaming"
                           + QDir::separator() + "tox")
           + QDir::separator();
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "Library" + QDir::separator()
                           + "Application Support" + QDir::separator() + "Tox")
           + QDir::separator();
#else
    /*
     * TODO: Change QStandardPaths::DataLocation to AppDataLocation when upgrate Qt to 5.4+
     * For now we need support Qt 5.3, so we use deprecated DataLocation
     * BTW, it's not a big deal since for linux AppDataLocation and DataLocation are equal
     */
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
           + QDir::separator();
#endif
}

/**
 * @brief Get path to directory, where the application cache are stored.
 * @return Path to application cache, ends with a directory separator.
 * @note To be removed when path migration is complete.
 */
QString Paths::getAppCacheDirPath() const
{
    if (portable)
        return qApp->applicationDirPath() + QDir::separator();

// workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "AppData" + QDir::separator() + "Roaming"
                           + QDir::separator() + "tox")
           + QDir::separator();
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "Library" + QDir::separator()
                           + "Application Support" + QDir::separator() + "Tox")
           + QDir::separator();
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
           + QDir::separator();
#endif
}

QString Paths::getExampleNodesFilePath() const
{
    QDir dir(getSettingsDirPath());
    constexpr static char nodesFileName[] = "bootstrapNodes.example.json";
    return dir.filePath(nodesFileName);
}

QString Paths::getUserNodesFilePath() const
{
    QDir dir(getSettingsDirPath());
    constexpr static char nodesFileName[] = "bootstrapNodes.json";
    return dir.filePath(nodesFileName);
}

QString Paths::getBackupUserNodesFilePath() const
{
    QDir dir(getSettingsDirPath());
    constexpr static char nodesFileName[] = "bootstrapNodes.backup.json";
    return dir.filePath(nodesFileName);
}

#endif // PATHS_VERSION_TCS_COMPLIANT
