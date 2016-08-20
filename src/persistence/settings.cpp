/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is free software: you can redistribute it and/or modify
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

#include "settings.h"
#include "src/persistence/smileypack.h"
#include "src/persistence/db/plaindb.h"
#include "src/core/corestructs.h"
#include "src/core/core.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
#include "src/persistence/profilelocker.h"
#include "src/persistence/settingsserializer.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#ifdef QTOX_PLATFORM_EXT
#include "src/platform/autorun.h"
#endif
#include "src/ipc.h"

#include <QFont>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <QList>
#include <QStyleFactory>
#include <QCryptographicHash>
#include <QMutexLocker>
#include <QThread>
#include <QNetworkProxy>

/**
 * @var QHash<QString, QByteArray> Settings::widgetSettings
 * @brief Assume all widgets have unique names
 * @warning Don't use it to save every single thing you want to save, use it
 * for some general purpose widgets, such as MainWindows or Splitters,
 * which have widget->saveX() and widget->loadX() methods.
 *
 * @var QString Settings::toxmeInfo
 * @brief Toxme info like name@server
 */

const QString Settings::globalSettingsFile = "qtox.ini";
Settings* Settings::settings{nullptr};
QMutex Settings::bigLock{QMutex::Recursive};
QThread* Settings::settingsThread{nullptr};

Settings::Settings() :
    loaded(false), useCustomDhtList{false},
    makeToxPortable{false}, currentProfileId(0)
{
    settingsThread = new QThread();
    settingsThread->setObjectName("qTox Settings");
    settingsThread->start(QThread::LowPriority);
    moveToThread(settingsThread);
    loadGlobal();
}

Settings::~Settings()
{
    sync();
    settingsThread->exit(0);
    settingsThread->wait();
    delete settingsThread;
}

/**
 * @brief Returns the singleton instance.
 */
Settings& Settings::getInstance()
{
    if (!settings)
        settings = new Settings();

    return *settings;
}

void Settings::destroyInstance()
{
    delete settings;
    settings = nullptr;
}

void Settings::loadGlobal()
{
    QMutexLocker locker{&bigLock};

    if (loaded)
        return;

    createSettingsDir();

    if (QFile(qApp->applicationDirPath()+QDir::separator()+globalSettingsFile).exists())
    {
        QSettings ps(qApp->applicationDirPath()+QDir::separator()+globalSettingsFile, QSettings::IniFormat);
        ps.setIniCodec("UTF-8");
        ps.beginGroup("General");
            makeToxPortable = ps.value("makeToxPortable", false).toBool();
        ps.endGroup();
    }
    else
    {
        makeToxPortable = false;
    }

    QDir dir(getSettingsDirPath());
    QString filePath = dir.filePath(globalSettingsFile);

    // If no settings file exist -- use the default one
    if (!QFile(filePath).exists())
    {
        qDebug() << "No settings file found, using defaults";
        filePath = ":/conf/" + globalSettingsFile;
    }

    qDebug() << "Loading settings from " + filePath;

    QSettings s(filePath, QSettings::IniFormat);
    s.setIniCodec("UTF-8");
    s.beginGroup("Login");
        autoLogin = s.value("autoLogin", false).toBool();
    s.endGroup();

    s.beginGroup("DHT Server");
        if (s.value("useCustomList").toBool())
        {
            useCustomDhtList = true;
            qDebug() << "Using custom bootstrap nodes list";
            int serverListSize = s.beginReadArray("dhtServerList");
            for (int i = 0; i < serverListSize; i ++)
            {
                s.setArrayIndex(i);
                DhtServer server;
                server.name = s.value("name").toString();
                server.userId = s.value("userId").toString();
                server.address = s.value("address").toString();
                server.port = static_cast<quint16>(s.value("port").toUInt());
                dhtServerList << server;
            }
            s.endArray();
        }
        else
        {
            useCustomDhtList=false;
        }
    s.endGroup();

    s.beginGroup("General");
        enableIPv6 = s.value("enableIPv6", true).toBool();
        translation = s.value("translation", "en").toString();
        showSystemTray = s.value("showSystemTray", true).toBool();
        makeToxPortable = s.value("makeToxPortable", false).toBool();
        autostartInTray = s.value("autostartInTray", false).toBool();
        closeToTray = s.value("closeToTray", false).toBool();
        forceTCP = s.value("forceTCP", false).toBool();
        proxyType = static_cast<ProxyType>(s.value("proxyType", 0).toInt());
        proxyAddr = s.value("proxyAddr", "").toString();
        proxyPort = static_cast<quint16>(s.value("proxyPort", 0).toUInt());
        if (currentProfile.isEmpty())
        {
            currentProfile = s.value("currentProfile", "").toString();
            currentProfileId = makeProfileId(currentProfile);
        }
        autoAwayTime = s.value("autoAwayTime", 10).toInt();
        checkUpdates = s.value("checkUpdates", true).toBool();
        showWindow = s.value("showWindow", true).toBool();
        showInFront = s.value("showInFront", false).toBool();
        notifySound = s.value("notifySound", true).toBool();
        busySound = s.value("busySound", false).toBool();
        groupAlwaysNotify = s.value("groupAlwaysNotify", false).toBool();
        fauxOfflineMessaging = s.value("fauxOfflineMessaging", true).toBool();
        autoSaveEnabled = s.value("autoSaveEnabled", false).toBool();
        globalAutoAcceptDir = s.value("globalAutoAcceptDir",
                                      QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)
                                      ).toString();
        separateWindow = s.value("separateWindow", false).toBool();
        dontGroupWindows = s.value("dontGroupWindows", true).toBool();
        groupchatPosition = s.value("groupchatPosition", true).toBool();
        stylePreference = static_cast<StyleType>(s.value("stylePreference", 1).toInt());
    s.endGroup();

    s.beginGroup("Advanced");
        Db::syncType sType = static_cast<Db::syncType>(s.value("dbSyncType", 2).toInt());
        setDbSyncType(sType);
    s.endGroup();

    s.beginGroup("Widgets");
        QList<QString> objectNames = s.childKeys();
        for (const QString& name : objectNames)
            widgetSettings[name] = s.value(name).toByteArray();

    s.endGroup();

    s.beginGroup("GUI");
        const QString DEFAULT_SMILEYS = ":/smileys/emojione/emoticons.xml";
        smileyPack = s.value("smileyPack", DEFAULT_SMILEYS).toString();
        if (!SmileyPack::isValid(smileyPack))
        {
            smileyPack = DEFAULT_SMILEYS;
        }
        emojiFontPointSize = s.value("emojiFontPointSize", 16).toInt();
        firstColumnHandlePos = s.value("firstColumnHandlePos", 50).toInt();
        secondColumnHandlePosFromRight = s.value("secondColumnHandlePosFromRight", 50).toInt();
        timestampFormat = s.value("timestampFormat", "hh:mm:ss").toString();
        dateFormat = s.value("dateFormat", "dddd, MMMM d, yyyy").toString();
        minimizeOnClose = s.value("minimizeOnClose", false).toBool();
        minimizeToTray = s.value("minimizeToTray", false).toBool();
        lightTrayIcon = s.value("lightTrayIcon", false).toBool();
        useEmoticons = s.value("useEmoticons", true).toBool();
        statusChangeNotificationEnabled = s.value("statusChangeNotificationEnabled", false).toBool();
        themeColor = s.value("themeColor", 0).toInt();
        style = s.value("style", "").toString();
        if (style == "") // Default to Fusion if available, otherwise no style
        {
            if (QStyleFactory::keys().contains("Fusion"))
                style = "Fusion";
            else
                style = "None";
        }
    s.endGroup();

    s.beginGroup("Chat");
    {
        chatMessageFont = s.value("chatMessageFont", Style::getFont(Style::Big)).value<QFont>();
    }
    s.endGroup();

    s.beginGroup("State");
        windowGeometry = s.value("windowGeometry", QByteArray()).toByteArray();
        windowState = s.value("windowState", QByteArray()).toByteArray();
        splitterState = s.value("splitterState", QByteArray()).toByteArray();
        dialogGeometry = s.value("dialogGeometry", QByteArray()).toByteArray();
        dialogSplitterState = s.value("dialogSplitterState", QByteArray()).toByteArray();
        dialogSettingsGeometry = s.value("dialogSettingsGeometry", QByteArray()).toByteArray();
    s.endGroup();

    s.beginGroup("Audio");
        inDev = s.value("inDev", "").toString();
        audioInDevEnabled = s.value("audioInDevEnabled", true).toBool();
        outDev = s.value("outDev", "").toString();
        audioOutDevEnabled = s.value("audioOutDevEnabled", true).toBool();
        audioInGainDecibel = s.value("inGain", 0).toReal();
        outVolume = s.value("outVolume", 100).toInt();
    s.endGroup();

    s.beginGroup("Video");
        videoDev = s.value("videoDev", "").toString();
        camVideoRes = s.value("camVideoRes", QRect()).toRect();
        screenRegion = s.value("screenRegion", QRect()).toRect();
        screenGrabbed = s.value("screenGrabbed", false).toBool();
        camVideoFPS = static_cast<quint16>(s.value("camVideoFPS", 0).toUInt());
    s.endGroup();

    // Read the embedded DHT bootstrap nodes list if needed
    if (dhtServerList.isEmpty())
    {
        QSettings rcs(":/conf/settings.ini", QSettings::IniFormat);
        rcs.setIniCodec("UTF-8");
        rcs.beginGroup("DHT Server");
            int serverListSize = rcs.beginReadArray("dhtServerList");
            for (int i = 0; i < serverListSize; i ++)
            {
                rcs.setArrayIndex(i);
                DhtServer server;
                server.name = rcs.value("name").toString();
                server.userId = rcs.value("userId").toString();
                server.address = rcs.value("address").toString();
                server.port = static_cast<quint16>(rcs.value("port").toUInt());
                dhtServerList << server;
            }
            rcs.endArray();
        rcs.endGroup();
    }

    loaded = true;
}

void Settings::loadPersonal()
{
    Profile* profile = Nexus::getProfile();
    if (!profile)
    {
        qCritical() << "No active profile, couldn't load personal settings";
        return;
    }
    loadPersonal(profile);
}

void Settings::loadPersonal(Profile* profile)
{
    QMutexLocker locker{&bigLock};

    QDir dir(getSettingsDirPath());
    QString filePath = dir.filePath(globalSettingsFile);

    // load from a profile specific friend data list if possible
    QString tmp = dir.filePath(profile->getName() + ".ini");
    if (QFile(tmp).exists()) // otherwise, filePath remains the global file
        filePath = tmp;

    qDebug()<<"Loading personal settings from"<<filePath;

    SettingsSerializer ps(filePath, profile->getPassword());
    ps.load();
    friendLst.clear();

    ps.beginGroup("Privacy");
        typingNotification = ps.value("typingNotification", true).toBool();
        enableLogging = ps.value("enableLogging", true).toBool();
    ps.endGroup();

    ps.beginGroup("Friends");
        int size = ps.beginReadArray("Friend");
        friendLst.reserve(size);
        for (int i = 0; i < size; i ++)
        {
            ps.setArrayIndex(i);
            friendProp fp;
            fp.addr = ps.value("addr").toString();
            fp.alias = ps.value("alias").toString();
            fp.note = ps.value("note").toString();
            fp.autoAcceptDir = ps.value("autoAcceptDir").toString();
            fp.circleID = ps.value("circle", -1).toInt();

            if (getEnableLogging())
                fp.activity = ps.value("activity", QDate()).toDate();

            friendLst[ToxId(fp.addr).publicKey] = fp;
        }
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("Requests");
        size = ps.beginReadArray("Request");
        friendRequests.clear();
        friendRequests.reserve(size);
        for (int i = 0; i < size; i ++)
        {
            ps.setArrayIndex(i);
            Request request;
            request.address = ps.value("addr").toString();
            request.message = ps.value("message").toString();
            request.read = ps.value("read").toBool();
            friendRequests.push_back(request);
        }
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("General");
        compactLayout = ps.value("compactLayout", true).toBool();
    ps.endGroup();

    ps.beginGroup("Circles");
        size = ps.beginReadArray("Circle");
        circleLst.clear();
        circleLst.reserve(size);
        for (int i = 0; i < size; i ++)
        {
            ps.setArrayIndex(i);
            circleProp cp;
            cp.name = ps.value("name").toString();
            cp.expanded = ps.value("expanded", true).toBool();
            circleLst.push_back(cp);
        }
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("Toxme");
        toxmeInfo = ps.value("info", "").toString();
        toxmeBio  = ps.value("bio", "").toString();
        toxmePriv = ps.value("priv", false).toBool();
        toxmePass = ps.value("pass", "").toString();
    ps.endGroup();
}

void Settings::resetToDefault()
{
    // To stop saving
    loaded = false;

    // Remove file with profile settings
    QDir dir(getSettingsDirPath());
    Profile *profile = Nexus::getProfile();
    QString localPath = dir.filePath(profile->getName() + ".ini");
    QFile local(localPath);
    if (local.exists())
        local.remove();
}

/**
 * @brief Asynchronous, saves the global settings.
 */
void Settings::saveGlobal()
{
    if (QThread::currentThread() != settingsThread)
        return (void) QMetaObject::invokeMethod(&getInstance(), "saveGlobal");

    QMutexLocker locker{&bigLock};
    if (!loaded)
        return;

    QString path = getSettingsDirPath() + globalSettingsFile;
    qDebug() << "Saving global settings at " + path;

    QSettings s(path, QSettings::IniFormat);
    s.setIniCodec("UTF-8");

    s.clear();

    s.beginGroup("Login");
        s.setValue("autoLogin", autoLogin);
    s.endGroup();

    s.beginGroup("DHT Server");
        s.setValue("useCustomList", useCustomDhtList);
        s.beginWriteArray("dhtServerList", dhtServerList.size());
        for (int i = 0; i < dhtServerList.size(); i ++)
        {
            s.setArrayIndex(i);
            s.setValue("name", dhtServerList[i].name);
            s.setValue("userId", dhtServerList[i].userId);
            s.setValue("address", dhtServerList[i].address);
            s.setValue("port", dhtServerList[i].port);
        }
        s.endArray();
    s.endGroup();

    s.beginGroup("General");
        s.setValue("enableIPv6", enableIPv6);
        s.setValue("translation",translation);
        s.setValue("makeToxPortable",makeToxPortable);
        s.setValue("showSystemTray", showSystemTray);
        s.setValue("autostartInTray",autostartInTray);
        s.setValue("closeToTray", closeToTray);
        s.setValue("proxyType", static_cast<int>(proxyType));
        s.setValue("forceTCP", forceTCP);
        s.setValue("proxyAddr", proxyAddr);
        s.setValue("proxyPort", proxyPort);
        s.setValue("currentProfile", currentProfile);
        s.setValue("autoAwayTime", autoAwayTime);
        s.setValue("checkUpdates", checkUpdates);
        s.setValue("showWindow", showWindow);
        s.setValue("showInFront", showInFront);
        s.setValue("notifySound", notifySound);
        s.setValue("busySound", busySound);
        s.setValue("groupAlwaysNotify", groupAlwaysNotify);
        s.setValue("fauxOfflineMessaging", fauxOfflineMessaging);
        s.setValue("separateWindow", separateWindow);
        s.setValue("dontGroupWindows", dontGroupWindows);
        s.setValue("groupchatPosition", groupchatPosition);
        s.setValue("autoSaveEnabled", autoSaveEnabled);
        s.setValue("globalAutoAcceptDir", globalAutoAcceptDir);
        s.setValue("stylePreference", static_cast<int>(stylePreference));
    s.endGroup();

    s.beginGroup("Advanced");
        s.setValue("dbSyncType", static_cast<int>(dbSyncType));
    s.endGroup();

    s.beginGroup("Widgets");
    const QList<QString> widgetNames = widgetSettings.keys();
    for (const QString& name : widgetNames)
        s.setValue(name, widgetSettings.value(name));
    s.endGroup();

    s.beginGroup("GUI");
        s.setValue("smileyPack", smileyPack);
        s.setValue("emojiFontPointSize", emojiFontPointSize);
        s.setValue("firstColumnHandlePos", firstColumnHandlePos);
        s.setValue("secondColumnHandlePosFromRight", secondColumnHandlePosFromRight);
        s.setValue("timestampFormat", timestampFormat);
        s.setValue("dateFormat", dateFormat);
        s.setValue("minimizeOnClose", minimizeOnClose);
        s.setValue("minimizeToTray", minimizeToTray);
        s.setValue("lightTrayIcon", lightTrayIcon);
        s.setValue("useEmoticons", useEmoticons);
        s.setValue("themeColor", themeColor);
        s.setValue("style", style);
        s.setValue("statusChangeNotificationEnabled", statusChangeNotificationEnabled);
    s.endGroup();

    s.beginGroup("Chat");
    {
        s.setValue("chatMessageFont", chatMessageFont);
    }
    s.endGroup();

    s.beginGroup("State");
        s.setValue("windowGeometry", windowGeometry);
        s.setValue("windowState", windowState);
        s.setValue("splitterState", splitterState);
        s.setValue("dialogGeometry", dialogGeometry);
        s.setValue("dialogSplitterState", dialogSplitterState);
        s.setValue("dialogSettingsGeometry", dialogSettingsGeometry);
    s.endGroup();

    s.beginGroup("Audio");
        s.setValue("inDev", inDev);
        s.setValue("audioInDevEnabled", audioInDevEnabled);
        s.setValue("outDev", outDev);
        s.setValue("audioOutDevEnabled", audioOutDevEnabled);
        s.setValue("inGain", audioInGainDecibel);
        s.setValue("outVolume", outVolume);
    s.endGroup();

    s.beginGroup("Video");
        s.setValue("videoDev", videoDev);
        s.setValue("camVideoRes", camVideoRes);
        s.setValue("camVideoFPS", camVideoFPS);
        s.setValue("screenRegion", screenRegion);
        s.setValue("screenGrabbed", screenGrabbed);
    s.endGroup();
}

/**
 * @brief Asynchronous, saves the current profile.
 */
void Settings::savePersonal()
{
    savePersonal(Nexus::getProfile());
}

/**
 * @brief Asynchronous, saves the profile.
 * @param profile Profile to save.
 */
void Settings::savePersonal(Profile* profile)
{
    if (!profile)
    {
        qDebug() << "Could not save personal settings because there is no active profile";
        return;
    }
    savePersonal(profile->getName(), profile->getPassword());
}

void Settings::savePersonal(QString profileName, const QString &password)
{
    if (QThread::currentThread() != settingsThread)
        return (void) QMetaObject::invokeMethod(&getInstance(), "savePersonal",
                                                Q_ARG(QString, profileName), Q_ARG(QString, password));

    QMutexLocker locker{&bigLock};
    if (!loaded)
        return;

    QString path = getSettingsDirPath() + profileName + ".ini";

    qDebug() << "Saving personal settings at " << path;

    SettingsSerializer ps(path, password);
    ps.beginGroup("Friends");
        ps.beginWriteArray("Friend", friendLst.size());
        int index = 0;
        for (auto& frnd : friendLst)
        {
            ps.setArrayIndex(index);
            ps.setValue("addr", frnd.addr);
            ps.setValue("alias", frnd.alias);
            ps.setValue("note", frnd.note);
            ps.setValue("autoAcceptDir", frnd.autoAcceptDir);
            ps.setValue("circle", frnd.circleID);

            if (getEnableLogging())
                ps.setValue("activity", frnd.activity);

            ++index;
        }
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("Requests");
        ps.beginWriteArray("Request", friendRequests.size());
        index = 0;
        for (auto& request : friendRequests)
        {
            ps.setArrayIndex(index);
            ps.setValue("addr", request.address);
            ps.setValue("message", request.message);
            ps.setValue("read", request.read);

            ++index;
        }
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("General");
        ps.setValue("compactLayout", compactLayout);
    ps.endGroup();

    ps.beginGroup("Circles");
        ps.beginWriteArray("Circle", circleLst.size());
        index = 0;
        for (auto& circle : circleLst)
        {
            ps.setArrayIndex(index);
            ps.setValue("name", circle.name);
            ps.setValue("expanded", circle.expanded);
            index++;
        }
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("Privacy");
        ps.setValue("typingNotification", typingNotification);
        ps.setValue("enableLogging", enableLogging);
    ps.endGroup();

    ps.beginGroup("Toxme");
        ps.setValue("info", toxmeInfo);
        ps.setValue("bio", toxmeBio);
        ps.setValue("priv", toxmePriv);
        ps.setValue("pass", toxmePass);
    ps.endGroup();

    ps.save();
}

uint32_t Settings::makeProfileId(const QString& profile)
{
    QByteArray data = QCryptographicHash::hash(profile.toUtf8(), QCryptographicHash::Md5);
    const uint32_t* dwords = reinterpret_cast<const uint32_t*>(data.constData());
    return dwords[0] ^ dwords[1] ^ dwords[2] ^ dwords[3];
}

/**
 * @brief Get path to directory, where the settings files are stored.
 * @return Path to settings directory, ends with a directory separator.
 */
QString Settings::getSettingsDirPath() const
{
    QMutexLocker locker{&bigLock};
    if (makeToxPortable)
        return qApp->applicationDirPath()+QDir::separator();

    // workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator()
                           + "AppData" + QDir::separator() + "Roaming" + QDir::separator() + "tox")+QDir::separator();
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator()
                           + "Library" + QDir::separator() + "Application Support" + QDir::separator() + "Tox")+QDir::separator();
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                           + QDir::separator() + "tox")+QDir::separator();
#endif
}

/**
 * @brief Get path to directory, where the application data are stored.
 * @return Path to application data, ends with a directory separator.
 */
QString Settings::getAppDataDirPath() const
{
    QMutexLocker locker{&bigLock};
    if (makeToxPortable)
        return qApp->applicationDirPath()+QDir::separator();

    // workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator()
                           + "AppData" + QDir::separator() + "Roaming" + QDir::separator() + "tox")+QDir::separator();
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator()
                           + "Library" + QDir::separator() + "Application Support" + QDir::separator() + "Tox")+QDir::separator();
#else
    /*
     * TODO: Change QStandardPaths::DataLocation to AppDataLocation when upgrate Qt to 5.4+
     * For now we need support Qt 5.3, so we use deprecated DataLocation
     * BTW, it's not a big deal since for linux AppDataLocation and DataLocation are equal
     */
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation))+QDir::separator();
#endif
}

/**
 * @brief Get path to directory, where the application cache are stored.
 * @return Path to application cache, ends with a directory separator.
 */
QString Settings::getAppCacheDirPath() const
{
    QMutexLocker locker{&bigLock};
    if (makeToxPortable)
        return qApp->applicationDirPath()+QDir::separator();

    // workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator()
                           + "AppData" + QDir::separator() + "Roaming" + QDir::separator() + "tox")+QDir::separator();
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator()
                           + "Library" + QDir::separator() + "Application Support" + QDir::separator() + "Tox")+QDir::separator();
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))+QDir::separator();
#endif
}

const QList<DhtServer>& Settings::getDhtServerList() const
{
    QMutexLocker locker{&bigLock};
    return dhtServerList;
}

void Settings::setDhtServerList(const QList<DhtServer>& newDhtServerList)
{
    QMutexLocker locker{&bigLock};

    if (newDhtServerList != dhtServerList)
    {
        dhtServerList = newDhtServerList;
        emit dhtServerListChanged(dhtServerList);
    }
}

bool Settings::getEnableIPv6() const
{
    QMutexLocker locker{&bigLock};
    return enableIPv6;
}

void Settings::setEnableIPv6(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != enableIPv6)
    {
        enableIPv6 = newValue;
        emit enableIPv6Changed(enableIPv6);
    }
}

bool Settings::getMakeToxPortable() const
{
    QMutexLocker locker{&bigLock};
    return makeToxPortable;
}

void Settings::setMakeToxPortable(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != makeToxPortable)
    {
        QFile(getSettingsDirPath() + globalSettingsFile).remove();
        makeToxPortable = newValue;
        saveGlobal();

        emit makeToxPortableChanged(makeToxPortable);
    }
}

bool Settings::getAutorun() const
{
    QMutexLocker locker{&bigLock};

#ifdef QTOX_PLATFORM_EXT
    return Platform::getAutorun();
#else
    return false;
#endif
}

void Settings::setAutorun(bool newValue)
{
#ifdef QTOX_PLATFORM_EXT
    QMutexLocker locker{&bigLock};

    bool autorun = Platform::getAutorun();

    if (newValue != autorun)
    {
        Platform::setAutorun(newValue);
        emit autorunChanged(autorun);
    }
#else
    Q_UNUSED(newValue);
#endif
}

bool Settings::getAutostartInTray() const
{
    QMutexLocker locker{&bigLock};
    return autostartInTray;
}

QString Settings::getStyle() const
{
    QMutexLocker locker{&bigLock};
    return style;
}

void Settings::setStyle(const QString& newStyle)
{
    QMutexLocker locker{&bigLock};

    if (newStyle != style)
    {
        style = newStyle;
        emit styleChanged(style);
    }
}

bool Settings::getShowSystemTray() const
{
    QMutexLocker locker{&bigLock};
    return showSystemTray;
}

void Settings::setShowSystemTray(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != showSystemTray)
    {
        showSystemTray = newValue;
        emit showSystemTrayChanged(newValue);
    }
}

void Settings::setUseEmoticons(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != useEmoticons)
    {
        useEmoticons = newValue;
        emit useEmoticonsChanged(useEmoticons);
    }
}

bool Settings::getUseEmoticons() const
{
    QMutexLocker locker{&bigLock};
    return useEmoticons;
}

void Settings::setAutoSaveEnabled(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != autoSaveEnabled)
    {
        autoSaveEnabled = newValue;
        emit autoSaveEnabledChanged(autoSaveEnabled);
    }
}

bool Settings::getAutoSaveEnabled() const
{
    QMutexLocker locker{&bigLock};
    return autoSaveEnabled;
}

void Settings::setAutostartInTray(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != autostartInTray)
    {
        autostartInTray = newValue;
        emit autostartInTrayChanged(autostartInTray);
    }
}

bool Settings::getCloseToTray() const
{
    QMutexLocker locker{&bigLock};
    return closeToTray;
}

void Settings::setCloseToTray(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != closeToTray)
    {
        closeToTray = newValue;
        emit closeToTrayChanged(newValue);
    }
}

bool Settings::getMinimizeToTray() const
{
    QMutexLocker locker{&bigLock};
    return minimizeToTray;
}

void Settings::setMinimizeToTray(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != minimizeToTray)
    {
        minimizeToTray = newValue;
        emit minimizeToTrayChanged(minimizeToTray);
    }
}

bool Settings::getLightTrayIcon() const
{
    QMutexLocker locker{&bigLock};
    return lightTrayIcon;
}

void Settings::setLightTrayIcon(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != lightTrayIcon)
    {
        lightTrayIcon = newValue;
        emit lightTrayIconChanged(lightTrayIcon);
    }
}

bool Settings::getStatusChangeNotificationEnabled() const
{
    QMutexLocker locker{&bigLock};
    return statusChangeNotificationEnabled;
}

void Settings::setStatusChangeNotificationEnabled(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != statusChangeNotificationEnabled)
    {
        statusChangeNotificationEnabled = newValue;
        emit statusChangeNotificationEnabledChanged(statusChangeNotificationEnabled);
    }
}

bool Settings::getShowInFront() const
{
    QMutexLocker locker{&bigLock};
    return showInFront;
}

void Settings::setShowInFront(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != showInFront)
    {
        showInFront = newValue;
        emit showInFrontChanged(showInFront);
    }
}

bool Settings::getNotifySound() const
{
    QMutexLocker locker{&bigLock};
    return notifySound;
}

void Settings::setNotifySound(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != notifySound)
    {
        notifySound = newValue;
        emit notifySoundChanged(notifySound);
    }
}

bool Settings::getBusySound() const
{
    QMutexLocker locker{&bigLock};
    return busySound;
}

void Settings::setBusySound(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != busySound)
    {
        busySound = newValue;
        emit busySoundChanged(busySound);
    }
}

bool Settings::getGroupAlwaysNotify() const
{
    QMutexLocker locker{&bigLock};
    return groupAlwaysNotify;
}

void Settings::setGroupAlwaysNotify(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != groupAlwaysNotify)
    {
        groupAlwaysNotify = newValue;
        emit groupAlwaysNotifyChanged(groupAlwaysNotify);
    }
}

QString Settings::getTranslation() const
{
    QMutexLocker locker{&bigLock};
    return translation;
}

void Settings::setTranslation(const QString& newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != translation)
    {
        translation = newValue;
        emit translationChanged(translation);
    }
}

void Settings::deleteToxme()
{
    setToxmeInfo("");
    setToxmeBio("");
    setToxmePriv(false);
    setToxmePass("");
}

void Settings::setToxme(QString name, QString server, QString bio, bool priv, QString pass)
{
    setToxmeInfo(name + "@" + server);
    setToxmeBio(bio);
    setToxmePriv(priv);
    if (!pass.isEmpty())
        setToxmePass(pass);
}

QString Settings::getToxmeInfo() const
{
    QMutexLocker locker{&bigLock};
    return toxmeInfo;
}

void Settings::setToxmeInfo(const QString& info)
{
    QMutexLocker locker{&bigLock};

    if (info != toxmeInfo)
    {
        if (info.split("@").size() == 2)
        {
            toxmeInfo = info;
            emit toxmeInfoChanged(toxmeInfo);
        }
        else
        {
            qWarning() << info << "is not a valid toxme string -> value ignored.";
        }
    }
}

QString Settings::getToxmeBio() const
{
    QMutexLocker locker{&bigLock};
    return toxmeBio;
}

void Settings::setToxmeBio(const QString& bio)
{
    QMutexLocker locker{&bigLock};

    if (bio != toxmeBio)
    {
        toxmeBio = bio;
        emit toxmeBioChanged(toxmeBio);
    }
}

bool Settings::getToxmePriv() const
{
    QMutexLocker locker{&bigLock};
    return toxmePriv;
}

void Settings::setToxmePriv(bool priv)
{
    QMutexLocker locker{&bigLock};

    if (priv != toxmePriv)
    {
        toxmePriv = priv;
        emit toxmePrivChanged(toxmePriv);
    }
}

QString Settings::getToxmePass() const
{
    QMutexLocker locker{&bigLock};
    return toxmePass;
}

void Settings::setToxmePass(const QString& pass)
{
    QMutexLocker locker{&bigLock};

    if (pass != toxmePass)
    {
        toxmePass = pass;

        // password is not exposed for security reasons
        emit toxmePassChanged();
    }
}

bool Settings::getForceTCP() const
{
    QMutexLocker locker{&bigLock};
    return forceTCP;
}

void Settings::setForceTCP(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != forceTCP)
    {
        forceTCP = newValue;
        emit forceTCPChanged(forceTCP);
    }
}

QNetworkProxy Settings::getProxy() const
{
    QNetworkProxy proxy;
    switch(Settings::getProxyType())
    {
        case ProxyType::ptNone:
            proxy.setType(QNetworkProxy::NoProxy);
            break;
        case ProxyType::ptSOCKS5:
            proxy.setType(QNetworkProxy::Socks5Proxy);
            break;
        case ProxyType::ptHTTP:
            proxy.setType(QNetworkProxy::HttpProxy);
            break;
        default:
            proxy.setType(QNetworkProxy::NoProxy);
            qWarning() << "Invalid Proxy type, setting to NoProxy";
            break;
    }

    proxy.setHostName(Settings::getProxyAddr());
    proxy.setPort(Settings::getProxyPort());
    return proxy;
}

Settings::ProxyType Settings::getProxyType() const
{
    QMutexLocker locker{&bigLock};
    return proxyType;
}

void Settings::setProxyType(ProxyType newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != proxyType)
    {
        proxyType = newValue;
        emit proxyTypeChanged(proxyType);
    }
}

QString Settings::getProxyAddr() const
{
    QMutexLocker locker{&bigLock};
    return proxyAddr;
}

void Settings::setProxyAddr(const QString& newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != proxyAddr)
    {
        proxyAddr = newValue;
        emit proxyAddressChanged(proxyAddr);
    }
}

quint16 Settings::getProxyPort() const
{
    QMutexLocker locker{&bigLock};
    return proxyPort;
}

void Settings::setProxyPort(quint16 newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != proxyPort)
    {
        proxyPort = newValue;
        emit proxyPortChanged(proxyPort);
    }
}

QString Settings::getCurrentProfile() const
{
    QMutexLocker locker{&bigLock};
    return currentProfile;
}

uint32_t Settings::getCurrentProfileId() const
{
    QMutexLocker locker{&bigLock};
    return currentProfileId;
}

void Settings::setCurrentProfile(const QString& profile)
{
    QMutexLocker locker{&bigLock};

    if (profile != currentProfile)
    {
        currentProfile = profile;
        currentProfileId = makeProfileId(currentProfile);
        emit currentProfileChanged(currentProfile);
        emit currentProfileIdChanged(currentProfileId);
    }
}

bool Settings::getEnableLogging() const
{
    QMutexLocker locker{&bigLock};
    return enableLogging;
}

void Settings::setEnableLogging(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != enableLogging)
    {
        enableLogging = newValue;
        emit enableLoggingChanged(enableLogging);
    }
}

Db::syncType Settings::getDbSyncType() const
{
    QMutexLocker locker{&bigLock};
    return dbSyncType;
}

void Settings::setDbSyncType(Db::syncType newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != dbSyncType)
    {
        dbSyncType = newValue;
        emit dbSyncTypeChanged(dbSyncType);
    }
}

int Settings::getAutoAwayTime() const
{
    QMutexLocker locker{&bigLock};
    return autoAwayTime;
}

/**
 * @brief Sets how long the user may stay idle, before online status is set to "away".
 * @param[in] newValue  the user idle duration in minutes
 * @note Values < 0 default to 10 minutes.
 */
void Settings::setAutoAwayTime(int newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue < 0)
        newValue = 10;

    if (newValue != autoAwayTime)
    {
        autoAwayTime = newValue;
        emit autoAwayTimeChanged(autoAwayTime);
    }
}

QString Settings::getAutoAcceptDir(const ToxId& id) const
{
    QMutexLocker locker{&bigLock};
    QString key = id.publicKey;

    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->autoAcceptDir;

    return QString();
}

void Settings::setAutoAcceptDir(const ToxId &id, const QString& dir)
{
    QMutexLocker locker{&bigLock};
    QString key = id.publicKey;

    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        it->autoAcceptDir = dir;
    }
    else
    {
        updateFriendAddress(id.toString());
        setAutoAcceptDir(id, dir);
    }
}

QString Settings::getContactNote(const ToxId &id) const
{
    QMutexLocker locker{&bigLock};

    auto it = friendLst.find(id.publicKey);
    if (it != friendLst.end())
        return it->note;

    return QString();
}

void Settings::setContactNote(const ToxId &id, const QString& note)
{
    QMutexLocker locker{&bigLock};

    auto it = friendLst.find(id.publicKey);
    if (it != friendLst.end())
    {
        qDebug() << note;
        it->note = note;
    }
    else
    {
        updateFriendAddress(id.toString());
        setContactNote(id, note);
    }
}

QString Settings::getGlobalAutoAcceptDir() const
{
    QMutexLocker locker{&bigLock};
    return globalAutoAcceptDir;
}

void Settings::setGlobalAutoAcceptDir(const QString& newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != globalAutoAcceptDir)
    {
        globalAutoAcceptDir = newValue;
        emit globalAutoAcceptDirChanged(globalAutoAcceptDir);
    }
}

const QFont& Settings::getChatMessageFont() const
{
    QMutexLocker locker(&bigLock);
    return chatMessageFont;
}

void Settings::setChatMessageFont(const QFont& font)
{
    QMutexLocker locker(&bigLock);

    if (font != chatMessageFont)
    {
        chatMessageFont = font;
        emit chatMessageFontChanged(chatMessageFont);
    }
}

void Settings::setWidgetData(const QString& uniqueName, const QByteArray& data)
{
    QMutexLocker locker{&bigLock};

    if (!widgetSettings.contains(uniqueName) ||
        widgetSettings[uniqueName] != data)
    {
        widgetSettings[uniqueName] = data;
        emit widgetDataChanged(uniqueName);
    }
}

QByteArray Settings::getWidgetData(const QString& uniqueName) const
{
    QMutexLocker locker{&bigLock};
    return widgetSettings.value(uniqueName);
}

QString Settings::getSmileyPack() const
{
    QMutexLocker locker{&bigLock};
    return smileyPack;
}

void Settings::setSmileyPack(const QString& value)
{
    QMutexLocker locker{&bigLock};

    if (value != smileyPack)
    {
        smileyPack = value;
        emit smileyPackChanged(smileyPack);
    }
}

int Settings::getEmojiFontPointSize() const
{
    QMutexLocker locker{&bigLock};
    return emojiFontPointSize;
}

void Settings::setEmojiFontPointSize(int value)
{
    QMutexLocker locker{&bigLock};

    if (value != emojiFontPointSize)
    {
        emojiFontPointSize = value;
        emit emojiFontPointSizeChanged(emojiFontPointSize);
    }
}

const QString& Settings::getTimestampFormat() const
{
    QMutexLocker locker{&bigLock};
    return timestampFormat;
}

void Settings::setTimestampFormat(const QString& format)
{
    QMutexLocker locker{&bigLock};

    if (format != timestampFormat)
    {
        timestampFormat = format;
        emit timestampFormatChanged(timestampFormat);
    }
}

const QString& Settings::getDateFormat() const
{
    QMutexLocker locker{&bigLock};
    return dateFormat;
}

void Settings::setDateFormat(const QString& format)
{
    QMutexLocker locker{&bigLock};

    if (format != dateFormat)
    {
        dateFormat = format;
        emit dateFormatChanged(dateFormat);
    }
}

Settings::StyleType Settings::getStylePreference() const
{
    QMutexLocker locker{&bigLock};
    return stylePreference;
}

void Settings::setStylePreference(StyleType newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != stylePreference)
    {
        stylePreference = newValue;
        emit stylePreferenceChanged(stylePreference);
    }
}

QByteArray Settings::getWindowGeometry() const
{
    QMutexLocker locker{&bigLock};
    return windowGeometry;
}

void Settings::setWindowGeometry(const QByteArray& value)
{
    QMutexLocker locker{&bigLock};

    if (value != windowGeometry)
    {
        windowGeometry = value;
        emit windowGeometryChanged(windowGeometry);
    }
}

QByteArray Settings::getWindowState() const
{
    QMutexLocker locker{&bigLock};
    return windowState;
}

void Settings::setWindowState(const QByteArray& value)
{
    QMutexLocker locker{&bigLock};

    if (value != windowState)
    {
        windowState = value;
        emit windowStateChanged(windowState);
    }
}

bool Settings::getCheckUpdates() const
{
    QMutexLocker locker{&bigLock};
    return checkUpdates;
}

void Settings::setCheckUpdates(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != checkUpdates)
    {
        checkUpdates = newValue;
        emit checkUpdatesChanged(checkUpdates);
    }
}

bool Settings::getShowWindow() const
{
    QMutexLocker locker{&bigLock};
    return showWindow;
}

void Settings::setShowWindow(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != showWindow)
    {
        showWindow = newValue;
        emit showWindowChanged(showWindow);
    }
}

QByteArray Settings::getSplitterState() const
{
    QMutexLocker locker{&bigLock};
    return splitterState;
}

void Settings::setSplitterState(const QByteArray& value)
{
    QMutexLocker locker{&bigLock};

    if(value != splitterState)
    {
        splitterState = value;
        emit splitterStateChanged(splitterState);
    }
}

QByteArray Settings::getDialogGeometry() const
{
    QMutexLocker locker{&bigLock};
    return dialogGeometry;
}

void Settings::setDialogGeometry(const QByteArray& value)
{
    QMutexLocker locker{&bigLock};

    if (value != dialogGeometry)
    {
        dialogGeometry = value;
        emit dialogGeometryChanged(dialogGeometry);
    }
}

QByteArray Settings::getDialogSplitterState() const
{
    QMutexLocker locker{&bigLock};
    return dialogSplitterState;
}

void Settings::setDialogSplitterState(const QByteArray& value)
{
    QMutexLocker locker{&bigLock};

    if (value != dialogSplitterState)
    {
        dialogSplitterState = value;
        emit dialogSplitterStateChanged(dialogSplitterState);
    }
}

QByteArray Settings::getDialogSettingsGeometry() const
{
    QMutexLocker locker{&bigLock};
    return dialogSettingsGeometry;
}

void Settings::setDialogSettingsGeometry(const QByteArray& value)
{
    QMutexLocker locker{&bigLock};

    if (value != dialogSettingsGeometry)
    {
        dialogSettingsGeometry = value;
        emit dialogSettingsGeometryChanged(dialogSettingsGeometry);
    }
}

bool Settings::getMinimizeOnClose() const
{
    QMutexLocker locker{&bigLock};
    return minimizeOnClose;
}

void Settings::setMinimizeOnClose(bool newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != minimizeOnClose)
    {
        minimizeOnClose = newValue;
        emit minimizeOnCloseChanged(minimizeOnClose);
    }
}

bool Settings::getTypingNotification() const
{
    QMutexLocker locker{&bigLock};
    return typingNotification;
}

void Settings::setTypingNotification(bool enabled)
{
    QMutexLocker locker{&bigLock};

    if (enabled != typingNotification)
    {
        typingNotification = enabled;
        emit typingNotificationChanged(typingNotification);
    }
}

QString Settings::getInDev() const
{
    QMutexLocker locker{&bigLock};
    return inDev;
}

void Settings::setInDev(const QString& deviceSpecifier)
{
    QMutexLocker locker{&bigLock};

    if (deviceSpecifier != inDev)
    {
        inDev = deviceSpecifier;
        emit inDevChanged(inDev);
    }
}

bool Settings::getAudioInDevEnabled() const
{
    QMutexLocker locker(&bigLock);
    return audioInDevEnabled;
}

void Settings::setAudioInDevEnabled(bool enabled)
{
    QMutexLocker locker(&bigLock);

    if (enabled != audioInDevEnabled)
    {
        audioInDevEnabled = enabled;
        emit audioInDevEnabledChanged(enabled);
    }
}

qreal Settings::getAudioInGainDecibel() const
{
    QMutexLocker locker{&bigLock};
    return audioInGainDecibel;
}

void Settings::setAudioInGainDecibel(qreal dB)
{
    QMutexLocker locker{&bigLock};

    if (dB < audioInGainDecibel || dB > audioInGainDecibel)
    {
        audioInGainDecibel = dB;
        emit audioInGainDecibelChanged(audioInGainDecibel);
    }
}

QString Settings::getVideoDev() const
{
    QMutexLocker locker{&bigLock};
    return videoDev;
}

void Settings::setVideoDev(const QString& deviceSpecifier)
{
    QMutexLocker locker{&bigLock};

    if(deviceSpecifier != videoDev)
    {
        videoDev = deviceSpecifier;
        emit videoDevChanged(videoDev);
    }
}

QString Settings::getOutDev() const
{
    QMutexLocker locker{&bigLock};
    return outDev;
}

void Settings::setOutDev(const QString& deviceSpecifier)
{
    QMutexLocker locker{&bigLock};

    if (deviceSpecifier != outDev)
    {
        outDev = deviceSpecifier;
        emit outDevChanged(outDev);
    }
}

bool Settings::getAudioOutDevEnabled() const
{
    QMutexLocker locker(&bigLock);
    return audioOutDevEnabled;
}

void Settings::setAudioOutDevEnabled(bool enabled)
{
    QMutexLocker locker(&bigLock);

    if(enabled != audioOutDevEnabled)
    {
        audioOutDevEnabled = enabled;
        emit audioOutDevEnabledChanged(audioOutDevEnabled);
    }
}

int Settings::getOutVolume() const
{
    QMutexLocker locker{&bigLock};
    return outVolume;
}

void Settings::setOutVolume(int volume)
{
    QMutexLocker locker{&bigLock};

    if (volume != outVolume)
    {
        outVolume = volume;
        emit outVolumeChanged(outVolume);
    }
}

QRect Settings::getScreenRegion() const
{
    QMutexLocker locker (&bigLock);
    return screenRegion;
}

void Settings::setScreenRegion(const QRect& value)
{
    QMutexLocker locker{&bigLock};

    if (value != screenRegion)
    {
        screenRegion = value;
        emit screenRegionChanged(screenRegion);
    }
}

bool Settings::getScreenGrabbed() const
{
    QMutexLocker locker(&bigLock);
    return screenGrabbed;
}

void Settings::setScreenGrabbed(bool value)
{
    QMutexLocker locker{&bigLock};

    if (value != screenGrabbed)
    {
        screenGrabbed = value;
        emit screenGrabbedChanged(screenGrabbed);
    }
}

QRect Settings::getCamVideoRes() const
{
    QMutexLocker locker{&bigLock};
    return camVideoRes;
}

void Settings::setCamVideoRes(QRect newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != camVideoRes)
    {
        camVideoRes = newValue;
        emit camVideoResChanged(camVideoRes);
    }
}

unsigned short Settings::getCamVideoFPS() const
{
    QMutexLocker locker{&bigLock};
    return camVideoFPS;
}

void Settings::setCamVideoFPS(unsigned short newValue)
{
    QMutexLocker locker{&bigLock};

    if (newValue != camVideoFPS)
    {
        camVideoFPS = newValue;
        emit camVideoFPSChanged(camVideoFPS);
    }
}

QString Settings::getFriendAddress(const QString& publicKey) const
{
    QMutexLocker locker{&bigLock};
    QString key = ToxId(publicKey).publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->addr;

    return QString();
}

void Settings::updateFriendAddress(const QString& newAddr)
{
    QMutexLocker locker{&bigLock};
    QString key = ToxId(newAddr).publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        it->addr = newAddr;
    }
    else
    {
        friendProp fp;
        fp.addr = newAddr;
        fp.alias = "";
        fp.note = "";
        fp.autoAcceptDir = "";
        friendLst[newAddr] = fp;
    }
}

QString Settings::getFriendAlias(const ToxId& id) const
{
    QMutexLocker locker{&bigLock};
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->alias;

    return QString();
}

void Settings::setFriendAlias(const ToxId& id, const QString& alias)
{
    QMutexLocker locker{&bigLock};
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        it->alias = alias;
    }
    else
    {
        friendProp fp;
        fp.addr = key;
        fp.alias = alias;
        fp.note = "";
        fp.autoAcceptDir = "";
        friendLst[key] = fp;
    }
}

int Settings::getFriendCircleID(const ToxId& id) const
{
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->circleID;

    return -1;
}

void Settings::setFriendCircleID(const ToxId& id, int circleID)
{
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        it->circleID = circleID;
    }
    else
    {
        friendProp fp;
        fp.addr = key;
        fp.alias = "";
        fp.note = "";
        fp.autoAcceptDir = "";
        fp.circleID = circleID;
        friendLst[key] = fp;
    }
}

QDate Settings::getFriendActivity(const ToxId& id) const
{
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->activity;

    return QDate();
}

void Settings::setFriendActivity(const ToxId& id, const QDate& activity)
{
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        it->activity = activity;
    }
    else
    {
        friendProp fp;
        fp.addr = key;
        fp.alias = "";
        fp.note = "";
        fp.autoAcceptDir = "";
        fp.circleID = -1;
        fp.activity = activity;
        friendLst[key] = fp;
    }
}

void Settings::removeFriendSettings(const ToxId& id)
{
    QMutexLocker locker{&bigLock};
    QString key = id.publicKey;
    friendLst.remove(key);
}

bool Settings::getFauxOfflineMessaging() const
{
    QMutexLocker locker{&bigLock};
    return fauxOfflineMessaging;
}

void Settings::setFauxOfflineMessaging(bool value)
{
    QMutexLocker locker{&bigLock};

    if (value != fauxOfflineMessaging)
    {
        fauxOfflineMessaging = value;
        emit fauxOfflineMessagingChanged(fauxOfflineMessaging);
    }
}

bool Settings::getCompactLayout() const
{
    QMutexLocker locker{&bigLock};
    return compactLayout;
}

void Settings::setCompactLayout(bool value)
{
    QMutexLocker locker{&bigLock};

    if (value != compactLayout)
    {
        compactLayout = value;
        emit compactLayoutChanged(value);
    }
}

bool Settings::getSeparateWindow() const
{
    QMutexLocker locker{&bigLock};
    return separateWindow;
}

void Settings::setSeparateWindow(bool value)
{
    QMutexLocker locker{&bigLock};

    if (value != separateWindow)
    {
        separateWindow = value;
        emit separateWindowChanged(value);
    }
}

bool Settings::getDontGroupWindows() const
{
    QMutexLocker locker{&bigLock};
    return dontGroupWindows;
}

void Settings::setDontGroupWindows(bool value)
{
    QMutexLocker locker{&bigLock};

    if (value != dontGroupWindows)
    {
        dontGroupWindows = value;
        emit dontGroupWindowsChanged(dontGroupWindows);
    }
}

bool Settings::getGroupchatPosition() const
{
    QMutexLocker locker{&bigLock};
    return groupchatPosition;
}

void Settings::setGroupchatPosition(bool value)
{
    QMutexLocker locker{&bigLock};

    if (value != groupchatPosition)
    {
        groupchatPosition = value;
        emit groupchatPositionChanged(value);
    }
}

int Settings::getCircleCount() const
{
    return circleLst.size();
}

QString Settings::getCircleName(int id) const
{
    return circleLst[id].name;
}

void Settings::setCircleName(int id, const QString& name)
{
    circleLst[id].name = name;
    savePersonal();
}

int Settings::addCircle(const QString& name)
{
    circleProp cp;
    cp.expanded = false;

    if (name.isEmpty())
        cp.name = tr("Circle #%1").arg(circleLst.count() + 1);
    else
        cp.name = name;

    circleLst.append(cp);
    savePersonal();
    return circleLst.count() - 1;
}

bool Settings::getCircleExpanded(int id) const
{
    return circleLst[id].expanded;
}

void Settings::setCircleExpanded(int id, bool expanded)
{
    circleLst[id].expanded = expanded;
}

bool Settings::addFriendRequest(const QString& friendAddress, const QString& message)
{
    QMutexLocker locker{&bigLock};

    for (auto queued : friendRequests)
    {
       if (queued.address == friendAddress)
       {
           queued.message = message;
           queued.read = false;
           return false;
       }
    }

    Request request;
    request.address = friendAddress;
    request.message = message;
    request.read = false;

    friendRequests.push_back(request);
    return true;
}

unsigned int Settings::getUnreadFriendRequests() const
{
    QMutexLocker locker{&bigLock};
    unsigned int unreadFriendRequests = 0;
    for (auto request : friendRequests)
        if (!request.read)
            unreadFriendRequests++;

    return unreadFriendRequests;
}

Settings::Request Settings::getFriendRequest(int index) const
{
    QMutexLocker locker{&bigLock};
    return friendRequests.at(index);
}

int Settings::getFriendRequestSize() const
{
    QMutexLocker locker{&bigLock};
    return friendRequests.size();
}

void Settings::clearUnreadFriendRequests()
{
    QMutexLocker locker{&bigLock};

    for (auto& request : friendRequests)
        request.read = true;
}

void Settings::removeFriendRequest(int index)
{
    QMutexLocker locker{&bigLock};
    friendRequests.removeAt(index);
}

void Settings::readFriendRequest(int index)
{
    QMutexLocker locker{&bigLock};
    friendRequests[index].read = true;
}

int Settings::removeCircle(int id)
{
    // Replace index with last one and remove last one instead.
    // This gives you contiguous ids all the time.
    circleLst[id] = circleLst.last();
    circleLst.pop_back();
    savePersonal();
    return circleLst.count();
}

int Settings::getThemeColor() const
{
    QMutexLocker locker{&bigLock};
    return themeColor;
}

void Settings::setThemeColor(int value)
{
    QMutexLocker locker{&bigLock};

    if (value != themeColor)
    {
        themeColor = value;
        emit themeColorChanged(themeColor);
    }
}

bool Settings::getAutoLogin() const
{
    QMutexLocker locker{&bigLock};
    return autoLogin;
}

void Settings::setAutoLogin(bool state)
{
    QMutexLocker locker{&bigLock};

    if (state != autoLogin)
    {
        autoLogin = state;
        emit autoLoginChanged(autoLogin);
    }
}

/**
 * @brief Write a default personal .ini settings file for a profile.
 * @param basename Filename without extension to save settings.
 * @example If basename is "profile", settings will be saved in profile.ini
 */
void Settings::createPersonal(QString basename)
{
    QString path = getSettingsDirPath() + QDir::separator() + basename + ".ini";
    qDebug() << "Creating new profile settings in " << path;

    QSettings ps(path, QSettings::IniFormat);
    ps.setIniCodec("UTF-8");
    ps.beginGroup("Friends");
        ps.beginWriteArray("Friend", 0);
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("Privacy");
    ps.endGroup();
}

/**
 * @brief Creates a path to the settings dir, if it doesn't already exist
 */
void Settings::createSettingsDir()
{
    QString dir = Settings::getSettingsDirPath();
    QDir directory(dir);
    if (!directory.exists() && !directory.mkpath(directory.absolutePath()))
        qCritical() << "Error while creating directory " << dir;
}

/**
 * @brief Waits for all asynchronous operations to complete
 */
void Settings::sync()
{
    if (QThread::currentThread() != settingsThread)
    {
        QMetaObject::invokeMethod(&getInstance(), "sync", Qt::BlockingQueuedConnection);
        return;
    }

    QMutexLocker locker{&bigLock};
    qApp->processEvents();
}
