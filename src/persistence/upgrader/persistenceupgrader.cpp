#include "persistenceupgrader.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringBuilder>

#include <cassert>
#include <memory>

namespace {
    QLatin1Literal lastVersion{"1.17.0"};
}


bool PersistenceUpgrader::runUpgrade(bool portable)
{
    PersistenceUpgrader p{portable};
    if(!p.isUpgradeNeeded()) {
        // No upgrade needed
        return true;
    }

    if(p.doUpgrade()) {
        return true;
    }

    qCritical() << "Upgrade failed";
    return false;
}

PersistenceUpgrader::PersistenceUpgrader(bool portable)
    : portable{portable}
{

}

bool PersistenceUpgrader::isUpgradeNeeded()
{
    if (!portable && isVersion1_16_3()) {
        curVersion = "1.16.3";
    } else {
        curVersion = versionFromFile();
    }

    return curVersion < lastVersion;
}

bool PersistenceUpgrader::isVersion1_16_3() {
    if(QFile(getSettingsPath_1_17_0()).exists()) {
        return false;
    }

    if(QFile(getSettingsDir_1_16_3() % "qtox.ini").exists()) {
        return true;
    }

    // we shouldn't come here, if we do, error on the side that this is not v1.16.3
    return false;
}

// copied from qTox v1.16.3
// we need our own copy of this logic to prevent different paths when the
// main logic is updated
QString PersistenceUpgrader::getSettingsDir_1_16_3() const
{
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

bool PersistenceUpgrader::checkedMove(QString source, QString dest)
{
    QFile srcFile{source};

    if(!srcFile.exists()) {
        qCritical() << "Source file doesn't exist";
        return false;
    }

    if(!srcFile.rename(dest)) {
        qCritical() << "Couldn't rename file";
        return false;
    }

    return true;
}

QString PersistenceUpgrader::getSettingsPath_1_17_0() {
    const QString path{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};
    if (path.isEmpty()) {
        qDebug() << "Can't find writable location for settings file";
        return {};
    }

    const QString newPath{path % QDir::separator() % "qtox.ini"};
}

QString PersistenceUpgrader::versionFromFile() {

    QString settingsPath{};

    if(portable) {
        settingsPath = qApp->applicationDirPath() + QDir::separator() + "qtox.ini";
    } else {
        settingsPath = getSettingsPath_1_17_0();
    }

    if(!QFile(settingsPath).exists()) {
        return "1.16.3";
    }

    QSettings s(settingsPath, QSettings::IniFormat);
    s.setIniCodec("UTF-8");

    return s.value("Version", "1.16.3").toString();
}

bool PersistenceUpgrader::doUpgrade()
{
    assert(!curVersion.isEmpty());

    if(curVersion < "1.17.0" && !upgradeFrom_1_16_3()) {
        return false;
    }

    return true;
}

QString PersistenceUpgrader::getProfilesDir_1_17_0() const
{
    const QString basePath{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)};
    const QLatin1Literal profileFolder{"profiles"};
    return basePath % QDir::separator() % profileFolder % QDir::separator();
}

QString PersistenceUpgrader::getGlobalSettingsPath_1_17_0() const
{
    const QLatin1Literal globalSettingsFile{"qtox.ini"};
    const QString path{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};

    return path % QDir::separator() % globalSettingsFile;
}

bool PersistenceUpgrader::upgradeFrom_1_16_3()
{
    if(portable) {
        // portable didn't work correctly in 1.16.3 so there's nothing to upgrade
        return true;
    }

    QString oldBase{getSettingsPath_1_17_0() % QDir::separator() % "profiles" % QDir::separator()};
    QString newBase{getProfilesDir_1_17_0()};

    // move <profile>.ini and <profile.db>

    QDir oldDir{oldBase};
    oldDir.setNameFilters({"*.ini", "*.db"});
    oldDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
    QStringList files{oldDir.entryList()};

    for(auto file : files) {
        checkedMove(oldBase % QDir::separator() % file, newBase % QDir::separator() % file);
    }

    // move qtox.ini

    if(!checkedMove(oldBase % QDir::separator() % "qtox.ini", getGlobalSettingsPath_1_17_0())) {
        return false;
    }

    curVersion = "1.17.0";
    return true;
}



