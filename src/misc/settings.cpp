/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>

    This file is part of Tox Qt GUI.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "settings.h"
#include "smileypack.h"
#include "src/corestructs.h"
#include "src/misc/db/plaindb.h"
#include "src/core.h"
#include "src/widget/gui.h"
#ifdef QTOX_PLATFORM_EXT
#include "src/platform/autorun.h"
#endif
#include "src/ipc.h"

#include <QFont>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QList>
#include <QStyleFactory>
#include <QCryptographicHash>


#define SHOW_SYSTEM_TRAY_DEFAULT (bool) true

const QString Settings::OLDFILENAME = "settings.ini";
const QString Settings::FILENAME = "qtox.ini";
Settings* Settings::settings{nullptr};
bool Settings::makeToxPortable{false};

Settings::Settings() :
    loaded(false), useCustomDhtList{false}, currentProfileId(0)
{
    load();
}

Settings& Settings::getInstance()
{
    if (!settings)
        settings = new Settings();
    return *settings;
}

void Settings::switchProfile(const QString& profile)
{
    // Saves current profile as main profile if this instance is main instance
    setCurrentProfile(profile);
    save(false);

    // If this instance is not main instance previous save did not happen therefore
    // we manually set profile again and load profile settings
    setCurrentProfile(profile);
    load();
}

QString Settings::detectProfile()
{
    QDir dir(getSettingsDirPath());
    QString path, profile = getCurrentProfile();
    path = dir.filePath(profile + Core::TOX_EXT);
    QFile file(path);
    if (profile.isEmpty() || !file.exists())
    {
        setCurrentProfile("");
#if 1 // deprecation attempt
        // if the last profile doesn't exist, fall back to old "data"
        path = dir.filePath(Core::CONFIG_FILE_NAME);
        QFile file(path);
        if (file.exists())
            return path;
        else if (QFile(path = dir.filePath("tox_save")).exists()) // also import tox_save if no data
            return path;
        else
#endif
        {
            profile = askProfiles();
            if (profile.isEmpty())
                return "";
            else
            {
                switchProfile(profile);
                return dir.filePath(profile + Core::TOX_EXT);
            }
        }
    }
    else
        return path;
}

QList<QString> Settings::searchProfiles()
{
    QList<QString> out;
    QDir dir(getSettingsDirPath());
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setNameFilters(QStringList("*.tox"));
    for (QFileInfo file : dir.entryInfoList())
        out += file.completeBaseName();
    return out;
}

QString Settings::askProfiles()
{   // TODO: allow user to create new Tox ID, even if a profile already exists
    QList<QString> profiles = searchProfiles();
    if (profiles.empty()) return "";
    bool ok;
    QString profile = GUI::itemInputDialog(nullptr,
                                            tr("Choose a profile"),
                                            tr("Please choose which identity to use"),
                                            profiles,
                                            0, // which slot to start on
                                            false, // if the user can enter their own input
                                            &ok);
    if (!ok) // user cancelled
        return "";
    else
        return profile;
}


void Settings::load()
{
    if (loaded)
        return;

    if (QFile(FILENAME).exists())
    {
        QSettings ps(FILENAME, QSettings::IniFormat);
        ps.beginGroup("General");
            makeToxPortable = ps.value("makeToxPortable", false).toBool();
        ps.endGroup();
    }
    else if (QFile(OLDFILENAME).exists())
    {
        QSettings ps(OLDFILENAME, QSettings::IniFormat);
        ps.beginGroup("General");
            makeToxPortable = ps.value("makeToxPortable", false).toBool();
        ps.endGroup();
    }
    else
        makeToxPortable = false;

    QDir dir(getSettingsDirPath());
    QString filePath = dir.filePath(FILENAME);

    //if no settings file exist -- use the default one
    if (!QFile(filePath).exists())
    {
        if (!QFile(filePath = dir.filePath(OLDFILENAME)).exists())
        {
            qDebug() << "No settings file found, using defaults";
            filePath = ":/conf/" + FILENAME;
        }
    }

    qDebug() << "Settings: Loading from "<<filePath;

    QSettings s(filePath, QSettings::IniFormat);
    s.beginGroup("DHT Server");
        if (s.value("useCustomList").toBool())
        {
            useCustomDhtList = true;
            qDebug() << "Using custom bootstrap nodes list";
            int serverListSize = s.beginReadArray("dhtServerList");
            for (int i = 0; i < serverListSize; i ++) {
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
            useCustomDhtList=false;
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
        currentProfile = s.value("currentProfile", "").toString();
        currentProfileId = makeProfileId(currentProfile);
        autoAwayTime = s.value("autoAwayTime", 10).toInt();
        checkUpdates = s.value("checkUpdates", false).toBool();
        showWindow = s.value("showWindow", true).toBool();
        showInFront = s.value("showInFront", false).toBool();
        notifySound = s.value("notifySound", true).toBool();
        groupAlwaysNotify = s.value("groupAlwaysNotify", false).toBool();
        fauxOfflineMessaging = s.value("fauxOfflineMessaging", true).toBool();
        autoSaveEnabled = s.value("autoSaveEnabled", false).toBool();
        globalAutoAcceptDir = s.value("globalAutoAcceptDir",
                                      QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)
                                      ).toString();
        compactLayout = s.value("compactLayout", false).toBool();
        groupchatPosition = s.value("groupchatPosition", true).toBool();
    s.endGroup();

    s.beginGroup("Advanced");
        int sType = s.value("dbSyncType", static_cast<int>(Db::syncType::stFull)).toInt();
        setDbSyncType(sType);
    s.endGroup();

    s.beginGroup("Widgets");
        QList<QString> objectNames = s.childKeys();
        for (const QString& name : objectNames) {
            widgetSettings[name] = s.value(name).toByteArray();
        }
    s.endGroup();

    s.beginGroup("GUI");
        enableSmoothAnimation = s.value("smoothAnimation", true).toBool();
        smileyPack = s.value("smileyPack", ":/smileys/TwitterEmojiSVG/emoticons.xml").toString();
        customEmojiFont = s.value("customEmojiFont", true).toBool();
        emojiFontFamily = s.value("emojiFontFamily", "DejaVu Sans").toString();
        emojiFontPointSize = s.value("emojiFontPointSize", 16).toInt();
        firstColumnHandlePos = s.value("firstColumnHandlePos", 50).toInt();
        secondColumnHandlePosFromRight = s.value("secondColumnHandlePosFromRight", 50).toInt();
        timestampFormat = s.value("timestampFormat", "hh:mm").toString();
        minimizeOnClose = s.value("minimizeOnClose", false).toBool();
        minimizeToTray = s.value("minimizeToTray", false).toBool();
        lightTrayIcon = s.value("lightTrayIcon", false).toBool();
        useNativeStyle = s.value("nativeStyle", false).toBool();
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
    s.endGroup();

    s.beginGroup("Audio");
        inDev = s.value("inDev", "").toString();
        outDev = s.value("outDev", "").toString();
        filterAudio = s.value("filterAudio", false).toBool();
    s.endGroup();

    s.beginGroup("Video");
        camVideoRes = s.value("camVideoRes",QSize()).toSize();
    s.endGroup();

    // Read the embedded DHT bootsrap nodes list if needed
    if (dhtServerList.isEmpty())
    {
        qDebug() << "Using embeded bootstrap nodes list";
        QSettings rcs(":/conf/settings.ini", QSettings::IniFormat);
        rcs.beginGroup("DHT Server");
            int serverListSize = rcs.beginReadArray("dhtServerList");
            for (int i = 0; i < serverListSize; i ++) {
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

    if (!currentProfile.isEmpty()) // new profile in Core::switchConfiguration
    {
        // load from a profile specific friend data list if possible
        QString tmp = dir.filePath(currentProfile + ".ini");
        if (QFile(tmp).exists()) // otherwise, filePath remains the global file
            filePath = tmp;

        QSettings ps(filePath, QSettings::IniFormat);
        friendLst.clear();
        ps.beginGroup("Friends");
            int size = ps.beginReadArray("Friend");
            for (int i = 0; i < size; i ++)
            {
                ps.setArrayIndex(i);
                friendProp fp;
                fp.addr = ps.value("addr").toString();
                fp.alias = ps.value("alias").toString();
                fp.autoAcceptDir = ps.value("autoAcceptDir").toString();
                friendLst[ToxID::fromString(fp.addr).publicKey] = fp;
            }
            ps.endArray();
        ps.endGroup();

        ps.beginGroup("Privacy");
            typingNotification = ps.value("typingNotification", false).toBool();
            enableLogging = ps.value("enableLogging", false).toBool();
            encryptLogs = ps.value("encryptLogs", false).toBool();
            encryptTox = ps.value("encryptTox", false).toBool();
        ps.endGroup();
    }
}

void Settings::save(bool writePersonal)
{
    QString filePath = QDir(getSettingsDirPath()).filePath(FILENAME);
    save(filePath, writePersonal);
}

void Settings::save(QString path, bool writePersonal)
{
#ifndef Q_OS_ANDROID
    if (IPC::getInstance().isCurrentOwner())
#endif
        saveGlobal(path);

    if (writePersonal) // Core::switchConfiguration
        savePersonal(path);
}

void Settings::saveGlobal(QString path)
{
    qDebug() << "Settings: Saving in " << path;

    QSettings s(path, QSettings::IniFormat);

    s.clear();

    s.beginGroup("DHT Server");
        s.setValue("useCustomList", useCustomDhtList);
        s.beginWriteArray("dhtServerList", dhtServerList.size());
        for (int i = 0; i < dhtServerList.size(); i ++) {
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
        s.setValue("compactLayout", compactLayout);
        s.setValue("groupchatPosition", groupchatPosition);
        s.setValue("autoSaveEnabled", autoSaveEnabled);
        s.setValue("globalAutoAcceptDir", globalAutoAcceptDir);
    s.endGroup();

    s.beginGroup("Advanced");
        s.setValue("dbSyncType", static_cast<int>(dbSyncType));
    s.endGroup();

    s.beginGroup("Widgets");
    const QList<QString> widgetNames = widgetSettings.keys();
    for (const QString& name : widgetNames) {
        s.setValue(name, widgetSettings.value(name));
    }
    s.endGroup();

    s.beginGroup("GUI");
        s.setValue("smoothAnimation", enableSmoothAnimation);
        s.setValue("smileyPack", smileyPack);
        s.setValue("customEmojiFont", customEmojiFont);
        s.setValue("emojiFontFamily", emojiFontFamily);
        s.setValue("emojiFontPointSize", emojiFontPointSize);
        s.setValue("firstColumnHandlePos", firstColumnHandlePos);
        s.setValue("secondColumnHandlePosFromRight", secondColumnHandlePosFromRight);
        s.setValue("timestampFormat", timestampFormat);
        s.setValue("minimizeOnClose", minimizeOnClose);
        s.setValue("minimizeToTray", minimizeToTray);
        s.setValue("lightTrayIcon", lightTrayIcon);
        s.setValue("nativeStyle", useNativeStyle);
        s.setValue("useEmoticons", useEmoticons);
        s.setValue("themeColor", themeColor);
        s.setValue("style", style);
        s.setValue("statusChangeNotificationEnabled", statusChangeNotificationEnabled);
    s.endGroup();

    s.beginGroup("State");
        s.setValue("windowGeometry", windowGeometry);
        s.setValue("windowState", windowState);
        s.setValue("splitterState", splitterState);
    s.endGroup();

    s.beginGroup("Audio");
        s.setValue("inDev", inDev);
        s.setValue("outDev", outDev);
        s.setValue("filterAudio", filterAudio);
    s.endGroup();

    s.beginGroup("Video");
        s.setValue("camVideoRes",camVideoRes);
    s.endGroup();
}

void Settings::savePersonal(QString path)
{
    if (currentProfile.isEmpty())
    {
        qDebug() << "Settings: could not save personal settings because currentProfile profile is empty";
        return;
    }

    qDebug() << "Settings: Saving personal in " << path;

    QSettings ps(QFileInfo(path).dir().filePath(currentProfile + ".ini"), QSettings::IniFormat);
    ps.beginGroup("Friends");
        ps.beginWriteArray("Friend", friendLst.size());
        int index = 0;
        for (auto& frnd : friendLst)
        {
            ps.setArrayIndex(index);
            ps.setValue("addr", frnd.addr);
            ps.setValue("alias", frnd.alias);
            ps.setValue("autoAcceptDir", frnd.autoAcceptDir);
            index++;
        }
        ps.endArray();
    ps.endGroup();

    ps.beginGroup("Privacy");
        ps.setValue("typingNotification", typingNotification);
        ps.setValue("enableLogging", enableLogging);
        ps.setValue("encryptLogs", encryptLogs);
        ps.setValue("encryptTox", encryptTox);
    ps.endGroup();
}

uint32_t Settings::makeProfileId(const QString& profile)
{
    QByteArray data = QCryptographicHash::hash(profile.toUtf8(), QCryptographicHash::Md5);
    const uint32_t* dwords = (uint32_t*)data.constData();
    return dwords[0] ^ dwords[1] ^ dwords[2] ^ dwords[3];
}

QString Settings::getSettingsDirPath()
{
    if (makeToxPortable)
        return ".";

    // workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "AppData" + QDir::separator() + "Roaming" + QDir::separator() + "tox");
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "tox");
#endif
}

QPixmap Settings::getSavedAvatar(const QString &ownerId)
{
    QDir dir(getSettingsDirPath());
    QString filePath = dir.filePath("avatars/"+ownerId.left(64)+".png");
    QFileInfo info(filePath);
    QPixmap pic;
    if (!info.exists())
    {
        QString filePath = dir.filePath("avatar_"+ownerId.left(64));
        if (!QFileInfo(filePath).exists()) // try without truncation, for old self avatars
            filePath = dir.filePath("avatar_"+ownerId);
        pic.load(filePath);
        saveAvatar(pic, ownerId);
        QFile::remove(filePath);
    }
    else
        pic.load(filePath);
    return pic;
}

void Settings::saveAvatar(QPixmap& pic, const QString& ownerId)
{
    QDir dir(getSettingsDirPath());
    dir.mkdir("avatars/");
    // ignore nospam (good idea, and also the addFriend funcs which call getAvatar don't have it)
    QString filePath = dir.filePath("avatars/"+ownerId.left(64)+".png");
    pic.save(filePath, "png");
}

void Settings::saveAvatarHash(const QByteArray& hash, const QString& ownerId)
{
    QDir dir(getSettingsDirPath());
    dir.mkdir("avatars/");
    QFile file(dir.filePath("avatars/"+ownerId.left(64)+".hash"));
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.write(hash);
    file.close();
}

QByteArray Settings::getAvatarHash(const QString& ownerId)
{
    QDir dir(getSettingsDirPath());
    dir.mkdir("avatars/");
    QFile file(dir.filePath("avatars/"+ownerId.left(64)+".hash"));
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    QByteArray out = file.readAll();
    file.close();
    return out;
}

const QList<Settings::DhtServer>& Settings::getDhtServerList() const
{
    return dhtServerList;
}

void Settings::setDhtServerList(const QList<DhtServer>& newDhtServerList)
{
    dhtServerList = newDhtServerList;
    emit dhtServerListChanged();
}

bool Settings::getEnableIPv6() const
{
    return enableIPv6;
}

void Settings::setEnableIPv6(bool newValue)
{
    enableIPv6 = newValue;
}

bool Settings::getMakeToxPortable() const
{
    return makeToxPortable;
}

void Settings::setMakeToxPortable(bool newValue)
{
    makeToxPortable = newValue;
    save(FILENAME); // Commit to the portable file that we don't want to use it
    if (!newValue) // Update the new file right now if not already done
        save();
}

bool Settings::getAutorun() const
{
#ifdef QTOX_PLATFORM_EXT
    return Platform::getAutorun();
#else
    return false;
#endif
}

void Settings::setAutorun(bool newValue)
{
#ifdef QTOX_PLATFORM_EXT
    Platform::setAutorun(newValue);
#else
    Q_UNUSED(newValue);
#endif
}

bool Settings::getAutostartInTray() const
{
    return autostartInTray;
}

QString Settings::getStyle() const
{
    return style;
}

void Settings::setStyle(const QString& newStyle) 
{
    style = newStyle;
}

bool Settings::getShowSystemTray() const
{
    return showSystemTray;
}

void Settings::setShowSystemTray(const bool& newValue)
{
    showSystemTray = newValue;
}

void Settings::setUseEmoticons(bool newValue)
{
    useEmoticons = newValue;
}

bool Settings::getUseEmoticons() const
{
    return useEmoticons;
}

void Settings::setAutoSaveEnabled(bool newValue)
{
    autoSaveEnabled = newValue;
}

bool Settings::getAutoSaveEnabled() const
{
    return autoSaveEnabled;
}

void Settings::setAutostartInTray(bool newValue)
{
    autostartInTray = newValue;
}

bool Settings::getCloseToTray() const
{
    return closeToTray;
}

void Settings::setCloseToTray(bool newValue)
{
    closeToTray = newValue;
}

bool Settings::getMinimizeToTray() const
{
    return minimizeToTray;
}


void Settings::setMinimizeToTray(bool newValue)
{
    minimizeToTray = newValue;
}

bool Settings::getLightTrayIcon() const
{
    return lightTrayIcon;
}

void Settings::setLightTrayIcon(bool newValue)
{
    lightTrayIcon = newValue;
}

bool Settings::getStatusChangeNotificationEnabled() const
{
    return statusChangeNotificationEnabled;
}

void Settings::setStatusChangeNotificationEnabled(bool newValue)
{
    statusChangeNotificationEnabled = newValue;
}

bool Settings::getShowInFront() const
{
   return showInFront;
}

void Settings::setShowInFront(bool newValue)
{
    showInFront = newValue;
}

bool Settings::getNotifySound() const
{
   return notifySound;
}

void Settings::setNotifySound(bool newValue)
{
    notifySound = newValue;
}

bool Settings::getGroupAlwaysNotify() const
{
    return groupAlwaysNotify;
}

void Settings::setGroupAlwaysNotify(bool newValue)
{
    groupAlwaysNotify = newValue;
}

QString Settings::getTranslation() const
{
    return translation;
}

void Settings::setTranslation(QString newValue)
{
    translation = newValue;
}

bool Settings::getForceTCP() const
{
    return forceTCP;
}

void Settings::setForceTCP(bool newValue)
{
    forceTCP = newValue;
}

ProxyType Settings::getProxyType() const
{
    return proxyType;
}

void Settings::setProxyType(int newValue)
{
    if (newValue >= 0 && newValue <= 2)
        proxyType = static_cast<ProxyType>(newValue);
    else
        proxyType = ProxyType::ptNone;
}

QString Settings::getProxyAddr() const
{
    return proxyAddr;
}

void Settings::setProxyAddr(const QString& newValue)
{
    proxyAddr = newValue;
}

int Settings::getProxyPort() const
{
    return proxyPort;
}

void Settings::setProxyPort(int newValue)
{
    proxyPort = newValue;
}

QString Settings::getCurrentProfile() const
{
    return currentProfile;
}

uint32_t Settings::getCurrentProfileId() const
{
    return currentProfileId;
}

void Settings::setCurrentProfile(QString profile)
{
    currentProfile = profile;
    currentProfileId = makeProfileId(currentProfile);
}

bool Settings::getEnableLogging() const
{
    return enableLogging;
}

void Settings::setEnableLogging(bool newValue)
{
    enableLogging = newValue;
}

bool Settings::getEncryptLogs() const
{
    return encryptLogs;
}

void Settings::setEncryptLogs(bool newValue)
{
    encryptLogs = newValue;
}

bool Settings::getEncryptTox() const
{
    return encryptTox;
}

void Settings::setEncryptTox(bool newValue)
{
    encryptTox = newValue;
}

Db::syncType Settings::getDbSyncType() const
{
    return dbSyncType;
}

void Settings::setDbSyncType(int newValue)
{
    if (newValue >= 0 && newValue <= 2)
        dbSyncType = static_cast<Db::syncType>(newValue);
    else
        dbSyncType = Db::syncType::stFull;
}

int Settings::getAutoAwayTime() const
{
    return autoAwayTime;
}

void Settings::setAutoAwayTime(int newValue)
{
    if (newValue < 0)
        newValue = 10;
    autoAwayTime = newValue;
}

QString Settings::getAutoAcceptDir(const ToxID& id) const
{
    QString key = id.publicKey;

    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        return it->autoAcceptDir;
    }

    return QString();
}

void Settings::setAutoAcceptDir(const ToxID &id, const QString& dir)
{
    QString key = id.publicKey;

    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        it->autoAcceptDir = dir;
    } else {
        updateFriendAdress(id.toString());
        setAutoAcceptDir(id, dir);
    }
}

QString Settings::getGlobalAutoAcceptDir() const
{
    return globalAutoAcceptDir;
}

void Settings::setGlobalAutoAcceptDir(const QString& newValue)
{
    globalAutoAcceptDir = newValue;
}

void Settings::setWidgetData(const QString& uniqueName, const QByteArray& data)
{
    widgetSettings[uniqueName] = data;
}

QByteArray Settings::getWidgetData(const QString& uniqueName) const
{
    return widgetSettings.value(uniqueName);
}

bool Settings::isAnimationEnabled() const
{
    return enableSmoothAnimation;
}

void Settings::setAnimationEnabled(bool newValue)
{
    enableSmoothAnimation = newValue;
}

QString Settings::getSmileyPack() const
{
    return smileyPack;
}

void Settings::setSmileyPack(const QString &value)
{
    smileyPack = value;
    emit smileyPackChanged();
}

bool Settings::isCurstomEmojiFont() const
{
    return customEmojiFont;
}

void Settings::setCurstomEmojiFont(bool value)
{
    customEmojiFont = value;
    emit emojiFontChanged();
}

int Settings::getEmojiFontPointSize() const
{
    return emojiFontPointSize;
}

void Settings::setEmojiFontPointSize(int value)
{
    emojiFontPointSize = value;
    emit emojiFontChanged();
}

int Settings::getFirstColumnHandlePos() const
{
    return firstColumnHandlePos;
}

void Settings::setFirstColumnHandlePos(const int pos)
{
    firstColumnHandlePos = pos;
}

int Settings::getSecondColumnHandlePosFromRight() const
{
    return secondColumnHandlePosFromRight;
}

void Settings::setSecondColumnHandlePosFromRight(const int pos)
{
    secondColumnHandlePosFromRight = pos;
}

const QString &Settings::getTimestampFormat() const
{
    return timestampFormat;
}

void Settings::setTimestampFormat(const QString &format)
{
    timestampFormat = format;
    emit timestampFormatChanged();
}

QString Settings::getEmojiFontFamily() const
{
    return emojiFontFamily;
}

void Settings::setEmojiFontFamily(const QString &value)
{
    emojiFontFamily = value;
    emit emojiFontChanged();
}

bool Settings::getUseNativeStyle() const
{
    return useNativeStyle;
}

void Settings::setUseNativeStyle(bool value)
{
    useNativeStyle = value;
}

QByteArray Settings::getWindowGeometry() const
{
    return windowGeometry;
}

void Settings::setWindowGeometry(const QByteArray &value)
{
    windowGeometry = value;
}

QByteArray Settings::getWindowState() const
{
    return windowState;
}

void Settings::setWindowState(const QByteArray &value)
{
    windowState = value;
}

bool Settings::getCheckUpdates() const
{
    return checkUpdates;
}

void Settings::setCheckUpdates(bool newValue)
{
    checkUpdates = newValue;
}

bool Settings::getShowWindow() const
{
    return showWindow;
}

void Settings::setShowWindow(bool newValue)
{
    showWindow = newValue;
}

QByteArray Settings::getSplitterState() const
{
    return splitterState;
}

void Settings::setSplitterState(const QByteArray &value)
{
    splitterState = value;
}

bool Settings::isMinimizeOnCloseEnabled() const
{
    return minimizeOnClose;
}

void Settings::setMinimizeOnClose(bool newValue)
{
    minimizeOnClose = newValue;
}

bool Settings::isTypingNotificationEnabled() const
{
    return typingNotification;
}

void Settings::setTypingNotification(bool enabled)
{
    typingNotification = enabled;
}

QString Settings::getInDev() const
{
    return inDev;
}

void Settings::setInDev(const QString& deviceSpecifier)
{
    inDev = deviceSpecifier;
}

QString Settings::getOutDev() const
{
    return outDev;
}

void Settings::setOutDev(const QString& deviceSpecifier)
{
    outDev = deviceSpecifier;
}

bool Settings::getFilterAudio() const
{
    return filterAudio;
}

void Settings::setFilterAudio(bool newValue)
{
    filterAudio = newValue;
}

QSize Settings::getCamVideoRes() const
{
    return camVideoRes;
}

void Settings::setCamVideoRes(QSize newValue)
{
    camVideoRes = newValue;
}

QString Settings::getFriendAdress(const QString &publicKey) const
{
    QString key = ToxID::fromString(publicKey).publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        return it->addr;
    }

    return QString();
}

void Settings::updateFriendAdress(const QString &newAddr)
{
    QString key = ToxID::fromString(newAddr).publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        it->addr = newAddr;
    } else {
        friendProp fp;
        fp.addr = newAddr;
        fp.alias = "";
        fp.autoAcceptDir = "";
        friendLst[newAddr] = fp;
    }
}

QString Settings::getFriendAlias(const ToxID &id) const
{
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        return it->alias;
    }

    return QString();
}

void Settings::setFriendAlias(const ToxID &id, const QString &alias)
{
    QString key = id.publicKey;
    auto it = friendLst.find(key);
    if (it != friendLst.end())
    {
        it->alias = alias;
    } else {
        friendProp fp;
        fp.addr = key;
        fp.alias = alias;
        fp.autoAcceptDir = "";
        friendLst[key] = fp;
    }
}

void Settings::removeFriendSettings(const ToxID &id)
{
    QString key = id.publicKey;
    friendLst.remove(key);
}

bool Settings::getFauxOfflineMessaging() const
{
    return fauxOfflineMessaging;
}

void Settings::setFauxOfflineMessaging(bool value)
{
    fauxOfflineMessaging = value;
}

bool Settings::getCompactLayout() const
{
    return compactLayout;
}

void Settings::setCompactLayout(bool value)
{
    compactLayout = value;
    emit compactLayoutChanged();
}

bool Settings::getGroupchatPosition() const
{
    return groupchatPosition;
}

void Settings::setGroupchatPosition(bool value)
{
    groupchatPosition = value;
}

int Settings::getThemeColor() const
{
    return themeColor;
}

void Settings::setThemeColor(const int &value)
{
    themeColor = value;
}
