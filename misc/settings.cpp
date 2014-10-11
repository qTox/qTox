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

#include <QFont>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QList>

const QString Settings::FILENAME = "settings.ini";
bool Settings::makeToxPortable{false};

Settings::Settings() :
    loaded(false), useCustomDhtList{false}
{
    load();
}

Settings::~Settings()
{
    save();
}

Settings& Settings::getInstance()
{
    static Settings settings;
    return settings;
}

void Settings::load()
{
    if (loaded) {
        return;
    }

    QFile portableSettings(FILENAME);
    if (portableSettings.exists())
    {
        QSettings ps(FILENAME, QSettings::IniFormat);
        ps.beginGroup("General");
            makeToxPortable = ps.value("makeToxPortable", false).toBool();
        ps.endGroup();
    }
    else
        makeToxPortable = false;

    QString filePath = QDir(getSettingsDirPath()).filePath(FILENAME);

    //if no settings file exist -- use the default one
    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "No settings file found, using defaults";
        filePath = ":/conf/" + FILENAME;
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

    friendAddresses.clear();
    s.beginGroup("Friends");
        int size = s.beginReadArray("fullAddresses");
        for (int i = 0; i < size; i ++) {
            s.setArrayIndex(i);
            friendAddresses.append(s.value("addr").toString());
        }
        s.endArray();
    s.endGroup();

    s.beginGroup("General");
        enableIPv6 = s.value("enableIPv6", true).toBool();
        useTranslations = s.value("useTranslations", true).toBool();
        makeToxPortable = s.value("makeToxPortable", false).toBool();
        autostartInTray = s.value("autostartInTray", false).toBool();
        forceTCP = s.value("forceTCP", false).toBool();
        useProxy = s.value("useProxy", false).toBool();
        proxyAddr = s.value("proxyAddr", "").toString();
        proxyPort = s.value("proxyPort", 0).toInt();
    s.endGroup();

    s.beginGroup("Widgets");
        QList<QString> objectNames = s.childKeys();
        for (const QString& name : objectNames) {
            widgetSettings[name] = s.value(name).toByteArray();
        }
    s.endGroup();

    s.beginGroup("GUI");
        enableSmoothAnimation = s.value("smoothAnimation", true).toBool();
        smileyPack = s.value("smileyPack", QString()).toString();
        customEmojiFont = s.value("customEmojiFont", true).toBool();
        emojiFontFamily = s.value("emojiFontFamily", "DejaVu Sans").toString();
        emojiFontPointSize = s.value("emojiFontPointSize", QApplication::font().pointSize()).toInt();
        firstColumnHandlePos = s.value("firstColumnHandlePos", 50).toInt();
        secondColumnHandlePosFromRight = s.value("secondColumnHandlePosFromRight", 50).toInt();
        timestampFormat = s.value("timestampFormat", "hh:mm").toString();
        minimizeOnClose = s.value("minimizeOnClose", false).toBool();
        useNativeStyle = s.value("nativeStyle", false).toBool();
    s.endGroup();

    s.beginGroup("State");
        windowGeometry = s.value("windowGeometry", QByteArray()).toByteArray();
        windowState = s.value("windowState", QByteArray()).toByteArray();
        splitterState = s.value("splitterState", QByteArray()).toByteArray();
    s.endGroup();

    s.beginGroup("Privacy");
        typingNotification = s.value("typingNotification", false).toBool();
        enableLogging = s.value("enableLogging", false).toBool();
        encryptLogs = s.value("encryptLogs", false).toBool();
    s.endGroup();

    // try to set a smiley pack if none is selected
    if (!SmileyPack::isValid(smileyPack) && !SmileyPack::listSmileyPacks().isEmpty())
        smileyPack = SmileyPack::listSmileyPacks()[0].second;

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
}

void Settings::save()
{
    QString filePath = QDir(getSettingsDirPath()).filePath(FILENAME);
    save(filePath);
}

void Settings::save(QString path)
{
    qDebug() << "Settings: Saving in "<<path;

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

    s.beginGroup("Friends");
        s.beginWriteArray("fullAddresses", friendAddresses.size());
        for (int i = 0; i < friendAddresses.size(); i ++) {
            s.setArrayIndex(i);
            s.setValue("addr", friendAddresses[i]);
        }
        s.endArray();
    s.endGroup();

    s.beginGroup("General");
        s.setValue("enableIPv6", enableIPv6);
        s.setValue("useTranslations",useTranslations);
        s.setValue("makeToxPortable",makeToxPortable);
        s.setValue("autostartInTray",autostartInTray);
        s.setValue("useProxy", useProxy);
        s.setValue("forceTCP", forceTCP);
        s.setValue("proxyAddr", proxyAddr);
        s.setValue("proxyPort", proxyPort);
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
        s.setValue("nativeStyle", useNativeStyle);
    s.endGroup();

    s.beginGroup("State");
        s.setValue("windowGeometry", windowGeometry);
        s.setValue("windowState", windowState);
        s.setValue("splitterState", splitterState);
    s.endGroup();

    s.beginGroup("Privacy");
        s.setValue("typingNotification", typingNotification);
        s.setValue("enableLogging", enableLogging);
        s.setValue("encryptLogs", encryptLogs);
    s.endGroup();
}

QString Settings::getSettingsDirPath()
{
    if (makeToxPortable)
        return ".";

    // workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
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

bool Settings::getAutostartInTray() const
{
    return autostartInTray;
}

void Settings::setAutostartInTray(bool newValue)
{
    autostartInTray = newValue;
}

bool Settings::getUseTranslations() const
{
    return useTranslations;
}

void Settings::setUseTranslations(bool newValue)
{
    useTranslations = newValue;
}

bool Settings::getForceTCP() const
{
    return forceTCP;
}

void Settings::setForceTCP(bool newValue)
{
    forceTCP = newValue;
}

bool Settings::getUseProxy() const
{
    return useProxy;
}
void Settings::setUseProxy(bool newValue)
{
    useProxy = newValue;
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
