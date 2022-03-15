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

#include "src/persistence/paths.h"

#include <ctime>

#include <QtTest/QtTest>
#include <QString>
#include <QStandardPaths>


class TestPaths : public QObject
{
    Q_OBJECT
private slots:
    void constructAuto();
    void constructPortable();
    void constructNonPortable();
#if PATHS_VERSION_TCS_COMPLIANT
    void checkPathsNonPortable();
    void checkPathsPortable();
#endif
private:
    static void verifyqToxPath(const QString& testPath, const QString& basePath, const QString& subPath);
};

namespace {
#if PATHS_VERSION_TCS_COMPLIANT
const QLatin1String globalSettingsFile{"qtox.ini"};
const QLatin1String profileFolder{"profiles"};
const QLatin1String themeFolder{"themes"};
const QLatin1String avatarsFolder{"avatars"};
const QLatin1String transfersFolder{"transfers"};
const QLatin1String screenshotsFolder{"screenshots"};
#endif // PATHS_VERSION_TCS_COMPLIANT
const QString sep{QDir::separator()};
}

/**
 * @brief Verifies construction in auto mode
 */
void TestPaths::constructAuto()
{
    Paths paths{Paths::Portable::Auto};
    // auto detection should succeed
    // the test environment should not provide a `qtox.ini`
    QVERIFY(paths.isPortable() == false);
}

/**
 * @brief Verifies construction in portable mode
 */
void TestPaths::constructPortable()
{
    Paths paths{Paths::Portable::Portable};
    // portable construction should succeed even though qtox.ini doesn't exist
    QVERIFY(paths.isPortable() == true);
}

/**
 * @brief Verifies construction in non-portable mode
 */
void TestPaths::constructNonPortable()
{
    Paths paths{Paths::Portable::NonPortable};
    // Non portable should succeed
    // the test environment should not provide a `qtox.ini`
    QVERIFY(paths.isPortable() == false);
}

/**
 * @brief Helper to verify qTox specific paths
 */
void TestPaths::verifyqToxPath(const QString& testPath, const QString& basePath, const QString& subPath)
{
    const QString expectPath = basePath % sep % subPath;
    QVERIFY(testPath == expectPath);
}

#if PATHS_VERSION_TCS_COMPLIANT
/**
 * @brief Check generated paths against expected values in non-portable mode
 */
void TestPaths::checkPathsNonPortable()
{
    Paths * paths = Paths::makePaths(Paths::Portable::NonPortable);
    QVERIFY(paths != nullptr);
    // Need non-portable environment to match our test cases
    QVERIFY(paths->isPortable() == false);

    // TODO(sudden6): write robust tests for Tox paths given by Tox Client Standard
    QString avatarDir{paths->getAvatarsDir()};
    QString toxSaveDir{paths->getToxSaveDir()};

    // avatars directory must be one level below toxsave to follow TCS
    QVERIFY(avatarDir.contains(toxSaveDir));

    const QString appData{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)};
    const QString appConfig{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};


    verifyqToxPath(paths->getProfilesDir(), appData, profileFolder % sep);
    verifyqToxPath(paths->getGlobalSettingsPath(), appConfig, globalSettingsFile);
    verifyqToxPath(paths->getScreenshotsDir(), appData, screenshotsFolder % sep);
    verifyqToxPath(paths->getTransfersDir(), appData, transfersFolder % sep);

    QStringList themeFolders{};
    themeFolders += QStandardPaths::locate(QStandardPaths::AppDataLocation, themeFolder,
                                           QStandardPaths::LocateDirectory);

    // look for themes beside the qTox binary with lowest priority
    const QString curPath{qApp->applicationDirPath()};
    themeFolders += curPath % sep % themeFolder % sep;

    QVERIFY(paths->getThemeDirs() == themeFolders);
}

/**
 * @brief Check generated paths against expected values in portable mode
 */
void TestPaths::checkPathsPortable()
{
    Paths * paths = Paths::makePaths(Paths::Portable::Portable);
    QVERIFY(paths != nullptr);
    // Need portable environment to match our test cases
    QVERIFY(paths->isPortable() == true);

    const QString basePath{qApp->applicationDirPath()};

    const QString avatarDir{paths->getAvatarsDir()};
    verifyqToxPath(avatarDir, basePath, profileFolder % sep % avatarsFolder % sep);

    const QString toxSaveDir{paths->getToxSaveDir()};
    verifyqToxPath(toxSaveDir, basePath, profileFolder % sep);

    // avatars directory must be one level below toxsave to follow TCS
    QVERIFY(avatarDir.contains(toxSaveDir));

    verifyqToxPath(paths->getProfilesDir(), basePath, profileFolder % sep);
    verifyqToxPath(paths->getGlobalSettingsPath(), basePath, globalSettingsFile);
    verifyqToxPath(paths->getScreenshotsDir(), basePath, screenshotsFolder % sep);
    verifyqToxPath(paths->getTransfersDir(), basePath, transfersFolder % sep);

    // look for themes beside the qTox binary with lowest priority
    const QString curPath{qApp->applicationDirPath()};
    QStringList themeFolders{curPath % sep % themeFolder % sep};

    QVERIFY(paths->getThemeDirs() == themeFolders);
}
#endif

QTEST_GUILESS_MAIN(TestPaths)
#include "paths_test.moc"
