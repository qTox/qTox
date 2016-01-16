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

#define SHOW_SYSTEM_TRAY_DEFAULT (bool) true

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

    if (QFile(globalSettingsFile).exists())
    {
        QSettings ps(globalSettingsFile, QSettings::IniFormat);
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
                server.port = s.value("port").toInt();
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
        showSystemTray = s.value("showSystemTray", SHOW_SYSTEM_TRAY_DEFAULT).toBool();
        makeToxPortable = s.value("makeToxPortable", false).toBool();
        autostartInTray = s.value("autostartInTray", false).toBool();
        closeToTray = s.value("closeToTray", false).toBool();
        forceTCP = s.value("forceTCP", false).toBool();
        setProxyType(s.value("proxyType", static_cast<int>(ProxyType::ptNone)).toInt());
        proxyAddr = s.value("proxyAddr", "").toString();
        proxyPort = s.value("proxyPort", 0).toInt();
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
        groupAlwaysNotify = s.value("groupAlwaysNotify", false).toBool();
        fauxOfflineMessaging = s.value("fauxOfflineMessaging", true).toBool();
        autoSaveEnabled = s.value("autoSaveEnabled", false).toBool();
        globalAutoAcceptDir = s.value("globalAutoAcceptDir",
                                      QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)
                                      ).toString();
        separateWindow = s.value("separateWindow", false).toBool();
        dontGroupWindows = s.value("dontGroupWindows", true).toBool();
        groupchatPosition = s.value("groupchatPosition", true).toBool();
    s.endGroup();

    s.beginGroup("Advanced");
        int sType = s.value("dbSyncType", static_cast<int>(Db::syncType::stFull)).toInt();
        setDbSyncType(sType);
    s.endGroup();

    s.beginGroup("Widgets");
        QList<QString> objectNames = s.childKeys();
        for (const QString& name : objectNames)
            widgetSettings[name] = s.value(name).toByteArray();

    s.endGroup();

    s.beginGroup("GUI");
        const QString DEFAULT_SMILEYS = ":/smileys/Universe/emoticons.xml";
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
        outDev = s.value("outDev", "").toString();
        inVolume = s.value("inVolume", 100).toInt();
        outVolume = s.value("outVolume", 100).toInt();
        filterAudio = s.value("filterAudio", false).toBool();
    s.endGroup();

    s.beginGroup("Video");
        videoDev = s.value("videoDev", "").toString();
        camVideoRes = s.value("camVideoRes",QSize()).toSize();
        camVideoFPS = s.value("camVideoFPS", 0).toUInt();
    s.endGroup();

    // Read the embedded DHT bootsrap nodes list if needed
    if (dhtServerList.isEmpty())
    {
        QSettings rcs(":/conf/settings.ini", QSettings::IniFormat);
        rcs.beginGroup("DHT Server");
            int serverListSize = rcs.beginReadArray("dhtServerList");
            for (int i = 0; i < serverListSize; i ++)
            {
                rcs.setArrayIndex(i);
                DhtServer server;
                server.name = rcs.value("name").toString();
                server.userId = rcs.value("userId").toString();
                server.address = rcs.value("address").toString();
                server.port = rcs.value("port").toInt();
                dhtServerList << server;
            }
            rcs.endArray();
        rcs.endGroup();
    }

    loaded = true;
}

void Settings::loadpersonal()
{
    Profile* profile = Nexus::getProfile();
    if (!profile)
    {
        qCritical() << "No active profile, couldn't load personal settings";
        return;
    }
    loadpersonal(profile);
}

void Settings::loadpersonal(Profile* profile)
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
}

void Settings::saveGlobal()
{
    if (QThread::currentThread() != settingsThread)
        return (void) QMetaObject::invokeMethod(&getInstance(), "saveGlobal");

    QMutexLocker locker{&bigLock};
    QString path = getSettingsDirPath() + globalSettingsFile;
    qDebug() << "Saving global settings at " + path;

    QSettings s(path, QSettings::IniFormat);

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
        s.setValue("groupAlwaysNotify", groupAlwaysNotify);
        s.setValue("fauxOfflineMessaging", fauxOfflineMessaging);
        s.setValue("separateWindow", separateWindow);
        s.setValue("dontGroupWindows", dontGroupWindows);
        s.setValue("groupchatPosition", groupchatPosition);
        s.setValue("autoSaveEnabled", autoSaveEnabled);
        s.setValue("globalAutoAcceptDir", globalAutoAcceptDir);
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
        s.setValue("outDev", outDev);
        s.setValue("inVolume", inVolume);
        s.setValue("outVolume", outVolume);
        s.setValue("filterAudio", filterAudio);
    s.endGroup();

    s.beginGroup("Video");
        s.setValue("videoDev", videoDev);
        s.setValue("camVideoRes",camVideoRes);
        s.setValue("camVideoFPS",camVideoFPS);
    s.endGroup();
}

void Settings::savePersonal()
{
    savePersonal(Nexus::getProfile());
}

void Settings::savePersonal(Profile* profile)
{
    if (!profile)
    {
        qDebug() << "Could not save personal settings because there is no active profile";
        return;
    }
    savePersonal(profile->getName(), profile->getPassword());
}

void Settings::savePersonal(QString profileName, QString password)
{
    if (QThread::currentThread() != settingsThread)
        return (void) QMetaObject::invokeMethod(&getInstance(), "savePersonal",
                                                Q_ARG(QString, profileName), Q_ARG(QString, password));

    QMutexLocker locker{&bigLock};

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

            index++;
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

    ps.save();
}

uint32_t Settings::makeProfileId(const QString& profile)
{
    QByteArray data = QCryptographicHash::hash(profile.toUtf8(), QCryptographicHash::Md5);
    const uint32_t* dwords = (uint32_t*)data.constData();
    return dwords[0] ^ dwords[1] ^ dwords[2] ^ dwords[3];
}

QString Settings::getSettingsDirPath()
{
    QMutexLocker locker{&bigLock};
    if (makeToxPortable)
        return QString(".")+QDir::separator();

    // workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator()
                           + "AppData" + QDir::separator() + "Roaming" + QDir::separator() + "tox")+QDir::separator();
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                           + QDir::separator() + "tox")+QDir::separator();
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
    dhtServerList = newDhtServerList;
    emit dhtServerListChanged();
}

bool Settings::getEnableIPv6() const
{
    QMutexLocker locker{&bigLock};
    return enableIPv6;
}

void Settings::setEnableIPv6(bool newValue)
{
    QMutexLocker locker{&bigLock};
    enableIPv6 = newValue;
}

bool Settings::getMakeToxPortable() const
{
    QMutexLocker locker{&bigLock};
    return makeToxPortable;
}

void Settings::setMakeToxPortable(bool newValue)
{
    QMutexLocker locker{&bigLock};
    QFile(getSettingsDirPath()+globalSettingsFile).remove();
    makeToxPortable = newValue;
    saveGlobal();
}

bool Settings::getAutorun() const
{
#ifdef QTOX_PLATFORM_EXT
    QMutexLocker locker{&bigLock};
    return Platform::getAutorun();
#else
    return false;
#endif
}

void Settings::setAutorun(bool newValue)
{
#ifdef QTOX_PLATFORM_EXT
    QMutexLocker locker{&bigLock};
    Platform::setAutorun(newValue);
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
    style = newStyle;
}

bool Settings::getShowSystemTray() const
{
    QMutexLocker locker{&bigLock};
    return showSystemTray;
}

void Settings::setShowSystemTray(const bool& newValue)
{
    QMutexLocker locker{&bigLock};
    showSystemTray = newValue;
}

void Settings::setUseEmoticons(bool newValue)
{
    QMutexLocker locker{&bigLock};
    useEmoticons = newValue;
}

bool Settings::getUseEmoticons() const
{
    QMutexLocker locker{&bigLock};
    return useEmoticons;
}

void Settings::setAutoSaveEnabled(bool newValue)
{
    QMutexLocker locker{&bigLock};
    autoSaveEnabled = newValue;
}

bool Settings::getAutoSaveEnabled() const
{
    QMutexLocker locker{&bigLock};
    return autoSaveEnabled;
}

void Settings::setAutostartInTray(bool newValue)
{
    QMutexLocker locker{&bigLock};
    autostartInTray = newValue;
}

bool Settings::getCloseToTray() const
{
    QMutexLocker locker{&bigLock};
    return closeToTray;
}

void Settings::setCloseToTray(bool newValue)
{
    QMutexLocker locker{&bigLock};
    closeToTray = newValue;
}

bool Settings::getMinimizeToTray() const
{
    QMutexLocker locker{&bigLock};
    return minimizeToTray;
}

void Settings::setMinimizeToTray(bool newValue)
{
    QMutexLocker locker{&bigLock};
    minimizeToTray = newValue;
}

bool Settings::getLightTrayIcon() const
{
    QMutexLocker locker{&bigLock};
    return lightTrayIcon;
}

void Settings::setLightTrayIcon(bool newValue)
{
    QMutexLocker locker{&bigLock};
    lightTrayIcon = newValue;
}

bool Settings::getStatusChangeNotificationEnabled() const
{
    QMutexLocker locker{&bigLock};
    return statusChangeNotificationEnabled;
}

void Settings::setStatusChangeNotificationEnabled(bool newValue)
{
    QMutexLocker locker{&bigLock};
    statusChangeNotificationEnabled = newValue;
}

bool Settings::getShowInFront() const
{
    QMutexLocker locker{&bigLock};
    return showInFront;
}

void Settings::setShowInFront(bool newValue)
{
    QMutexLocker locker{&bigLock};
    showInFront = newValue;
}

bool Settings::getNotifySound() const
{
    QMutexLocker locker{&bigLock};
    return notifySound;
}

void Settings::setNotifySound(bool newValue)
{
    QMutexLocker locker{&bigLock};
    notifySound = newValue;
}

bool Settings::getGroupAlwaysNotify() const
{
    QMutexLocker locker{&bigLock};
    return groupAlwaysNotify;
}

void Settings::setGroupAlwaysNotify(bool newValue)
{
    QMutexLocker locker{&bigLock};
    groupAlwaysNotify = newValue;
}

QString Settings::getTranslation() const
{
    QMutexLocker locker{&bigLock};
    return translation;
}

void Settings::setTranslation(QString newValue)
{
    QMutexLocker locker{&bigLock};
    translation = newValue;
}

bool Settings::getForceTCP() const
{
    QMutexLocker locker{&bigLock};
    return forceTCP;
}

void Settings::setForceTCP(bool newValue)
{
    QMutexLocker locker{&bigLock};
    forceTCP = newValue;
}

ProxyType Settings::getProxyType() const
{
    QMutexLocker locker{&bigLock};
    return proxyType;
}

void Settings::setProxyType(int newValue)
{
    QMutexLocker locker{&bigLock};
    if (newValue >= 0 && newValue <= 2)
        proxyType = static_cast<ProxyType>(newValue);
    else
        proxyType = ProxyType::ptNone;
}

QString Settings::getProxyAddr() const
{
    QMutexLocker locker{&bigLock};
    return proxyAddr;
}

void Settings::setProxyAddr(const QString& newValue)
{
    QMutexLocker locker{&bigLock};
    proxyAddr = newValue;
}

int Settings::getProxyPort() const
{
    QMutexLocker locker{&bigLock};
    return proxyPort;
}

void Settings::setProxyPort(int newValue)
{
    QMutexLocker locker{&bigLock};
    proxyPort = newValue;
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

void Settings::setCurrentProfile(QString profile)
{
    QMutexLocker locker{&bigLock};
    currentProfile = profile;
    currentProfileId = makeProfileId(currentProfile);
}

bool Settings::getEnableLogging() const
{
    QMutexLocker locker{&bigLock};
    return enableLogging;
}

void Settings::setEnableLogging(bool newValue)
{
    QMutexLocker locker{&bigLock};
    enableLogging = newValue;
}

Db::syncType Settings::getDbSyncType() const
{
    QMutexLocker locker{&bigLock};
    return dbSyncType;
}

void Settings::setDbSyncType(int newValue)
{
    QMutexLocker locker{&bigLock};
    if (newValue >= 0 && newValue <= 2)
        dbSyncType = static_cast<Db::syncType>(newValue);
    else
        dbSyncType = Db::syncType::stFull;
}

int Settings::getAutoAwayTime() const
{
    QMutexLocker locker{&bigLock};
    return autoAwayTime;
}

void Settings::setAutoAwayTime(int newValue)
{
    QMutexLocker locker{&bigLock};
    if (newValue < 0)
        newValue = 10;

    autoAwayTime = newValue;
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
        updateFriendAdress(id.toString());
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
        updateFriendAdress(id.toString());
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
    globalAutoAcceptDir = newValue;
}

void Settings::setWidgetData(const QString& uniqueName, const QByteArray& data)
{
    QMutexLocker locker{&bigLock};
    widgetSettings[uniqueName] = data;
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

void Settings::setSmileyPack(const QString &value)
{
    QMutexLocker locker{&bigLock};
    smileyPack = value;
    emit smileyPackChanged();
}

int Settings::getEmojiFontPointSize() const
{
    QMutexLocker locker{&bigLock};
    return emojiFontPointSize;
}

void Settings::setEmojiFontPointSize(int value)
{
    QMutexLocker locker{&bigLock};
    emojiFontPointSize = value;
    emit emojiFontChanged();
}

int Settings::getFirstColumnHandlePos() const
{
    QMutexLocker locker{&bigLock};
    return firstColumnHandlePos;
}

void Settings::setFirstColumnHandlePos(const int pos)
{
    QMutexLocker locker{&bigLock};
    firstColumnHandlePos = pos;
}

int Settings::getSecondColumnHandlePosFromRight() const
{
    QMutexLocker locker{&bigLock};
    return secondColumnHandlePosFromRight;
}

void Settings::setSecondColumnHandlePosFromRight(const int pos)
{
    QMutexLocker locker{&bigLock};
    secondColumnHandlePosFromRight = pos;
}

const QString& Settings::getTimestampFormat() const
{
    QMutexLocker locker{&bigLock};
    return timestampFormat;
}

void Settings::setTimestampFormat(const QString &format)
{
    QMutexLocker locker{&bigLock};
    timestampFormat = format;
}

const QString& Settings::getDateFormat() const
{
    QMutexLocker locker{&bigLock};
    return dateFormat;
}

void Settings::setDateFormat(const QString &format)
{
    QMutexLocker locker{&bigLock};
    dateFormat = format;
}

QByteArray Settings::getWindowGeometry() const
{
    QMutexLocker locker{&bigLock};
    return windowGeometry;
}

void Settings::setWindowGeometry(const QByteArray &value)
{
    QMutexLocker locker{&bigLock};
    windowGeometry = value;
}

QByteArray Settings::getWindowState() const
{
    QMutexLocker locker{&bigLock};
    return windowState;
}

void Settings::setWindowState(const QByteArray &value)
{
    QMutexLocker locker{&bigLock};
    windowState = value;
}

bool Settings::getCheckUpdates() const
{
    QMutexLocker locker{&bigLock};
    return checkUpdates;
}

void Settings::setCheckUpdates(bool newValue)
{
    QMutexLocker locker{&bigLock};
    checkUpdates = newValue;
}

bool Settings::getShowWindow() const
{
    QMutexLocker locker{&bigLock};
    return showWindow;
}

void Settings::setShowWindow(bool newValue)
{
    QMutexLocker locker{&bigLock};
    showWindow = newValue;
}

QByteArray Settings::getSplitterState() const
{
    QMutexLocker locker{&bigLock};
    return splitterState;
}

void Settings::setSplitterState(const QByteArray &value)
{
    QMutexLocker locker{&bigLock};
    splitterState = value;
}

QByteArray Settings::getDialogGeometry() const
{
    QMutexLocker locker{&bigLock};
    return dialogGeometry;
}

void Settings::setDialogGeometry(const QByteArray &value)
{
    QMutexLocker locker{&bigLock};
    dialogGeometry = value;
}

QByteArray Settings::getDialogSplitterState() const
{
    QMutexLocker locker{&bigLock};
    return dialogSplitterState;
}

void Settings::setDialogSplitterState(const QByteArray &value)
{
    QMutexLocker locker{&bigLock};
    dialogSplitterState = value;
}

QByteArray Settings::getDialogSettingsGeometry() const
{
    QMutexLocker locker{&bigLock};
    return dialogSettingsGeometry;
}

void Settings::setDialogSettingsGeometry(const QByteArray &value)
{
    QMutexLocker locker{&bigLock};
    dialogSettingsGeometry = value;
}

bool Settings::isMinimizeOnCloseEnabled() const
{
    QMutexLocker locker{&bigLock};
    return minimizeOnClose;
}

void Settings::setMinimizeOnClose(bool newValue)
{
    QMutexLocker locker{&bigLock};
    minimizeOnClose = newValue;
}

bool Settings::isTypingNotificationEnabled() const
{
    QMutexLocker locker{&bigLock};
    return typingNotification;
}

void Settings::setTypingNotification(bool enabled)
{
    QMutexLocker locker{&bigLock};
    typingNotification = enabled;
}

QString Settings::getInDev() const
{
    QMutexLocker locker{&bigLock};
    return inDev;
}

void Settings::setInDev(const QString& deviceSpecifier)
{
    QMutexLocker locker{&bigLock};
    inDev = deviceSpecifier;
}

int Settings::getInVolume() const
{
    QMutexLocker locker{&bigLock};
    return inVolume;
}

void Settings::setInVolume(int volume)
{
    QMutexLocker locker{&bigLock};
    inVolume = volume;
}

QString Settings::getVideoDev() const
{
    QMutexLocker locker{&bigLock};
    return videoDev;
}

void Settings::setVideoDev(const QString& deviceSpecifier)
{
    QMutexLocker locker{&bigLock};
    videoDev = deviceSpecifier;
}

QString Settings::getOutDev() const
{
    QMutexLocker locker{&bigLock};
    return outDev;
}

void Settings::setOutDev(const QString& deviceSpecifier)
{
    QMutexLocker locker{&bigLock};
    outDev = deviceSpecifier;
}

int Settings::getOutVolume() const
{
    QMutexLocker locker{&bigLock};
    return outVolume;
}

void Settings::setOutVolume(int volume)
{
    QMutexLocker locker{&bigLock};
    outVolume = volume;
}

bool Settings::getFilterAudio() const
{
    QMutexLocker locker{&bigLock};
    return filterAudio;
}

void Settings::setFilterAudio(bool newValue)
{
    QMutexLocker locker{&bigLock};
    filterAudio = newValue;
}

QSize Settings::getCamVideoRes() const
{
    QMutexLocker locker{&bigLock};
    return camVideoRes;
}

void Settings::setCamVideoRes(QSize newValue)
{
    QMutexLocker locker{&bigLock};
    camVideoRes = newValue;
}

unsigned short Settings::getCamVideoFPS() const
{
    QMutexLocker locker{&bigLock};
    return camVideoFPS;
}

void Settings::setCamVideoFPS(unsigned short newValue)
{
    QMutexLocker locker{&bigLock};
    camVideoFPS = newValue;
}

QString Settings::getFriendAdress(const QString &publicKey) const
{
    QMutexLocker locker{&bigLock};
    QString key = ToxId(publicKey).publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->addr;

    return QString();
}

void Settings::updateFriendAdress(const QString &newAddr)
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

QString Settings::getFriendAlias(const ToxId &id) const
{
    QMutexLocker locker{&bigLock};
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->alias;

    return QString();
}

void Settings::setFriendAlias(const ToxId &id, const QString &alias)
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

int Settings::getFriendCircleID(const ToxId &id) const
{
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->circleID;

    return -1;
}

void Settings::setFriendCircleID(const ToxId &id, int circleID)
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

QDate Settings::getFriendActivity(const ToxId &id) const
{
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
        return it->activity;

    return QDate();
}

void Settings::setFriendActivity(const ToxId &id, const QDate &activity)
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

void Settings::removeFriendSettings(const ToxId &id)
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
    fauxOfflineMessaging = value;
}

bool Settings::getCompactLayout() const
{
    QMutexLocker locker{&bigLock};
    return compactLayout;
}

void Settings::setCompactLayout(bool value)
{
    QMutexLocker locker{&bigLock};
    compactLayout = value;
}

bool Settings::getSeparateWindow() const
{
    QMutexLocker locker{&bigLock};
    return separateWindow;
}

void Settings::setSeparateWindow(bool value)
{
    QMutexLocker locker{&bigLock};
    separateWindow = value;
}

bool Settings::getDontGroupWindows() const
{
    QMutexLocker locker{&bigLock};
    return dontGroupWindows;
}

void Settings::setDontGroupWindows(bool value)
{
    QMutexLocker locker{&bigLock};
    dontGroupWindows = value;
}

bool Settings::getGroupchatPosition() const
{
    QMutexLocker locker{&bigLock};
    return groupchatPosition;
}

void Settings::setGroupchatPosition(bool value)
{
    QMutexLocker locker{&bigLock};
    groupchatPosition = value;
}

int Settings::getCircleCount() const
{
    return circleLst.size();
}

QString Settings::getCircleName(int id) const
{
    return circleLst[id].name;
}

void Settings::setCircleName(int id, const QString &name)
{
    circleLst[id].name = name;
    savePersonal();
}

int Settings::addCircle(const QString &name)
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

void Settings::setThemeColor(const int &value)
{
    QMutexLocker locker{&bigLock};
    themeColor = value;
}

bool Settings::getAutoLogin() const
{
    QMutexLocker locker{&bigLock};
    return autoLogin;
}

void Settings::setAutoLogin(bool state)
{
    QMutexLocker locker{&bigLock};
    autoLogin = state;
}

void Settings::createPersonal(QString basename)
{
    QString path = getSettingsDirPath() + QDir::separator() + basename + ".ini";
    qDebug() << "Creating new profile settings in " << path;

    QSettings ps(path, QSettings::IniFormat);
    ps.beginGroup("Friends");
        ps.beginWriteArray("Friend", 0);
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("Privacy");
    ps.endGroup();
}

void Settings::createSettingsDir()
{
    QString dir = Settings::getSettingsDirPath();
    QDir directory(dir);
    if (!directory.exists() && !directory.mkpath(directory.absolutePath()))
        qCritical() << "Error while creating directory " << dir;
}

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
