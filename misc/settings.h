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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QHash>
#include <QObject>
#include <QPixmap>

class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings& getInstance();
    ~Settings();

    void executeSettingsDialog(QWidget* parent);

    static QString getSettingsDirPath();

    struct DhtServer
    {
        QString name;
        QString userId;
        QString address;
        quint16 port;
    };

    const QList<DhtServer>& getDhtServerList() const;
    void setDhtServerList(const QList<DhtServer>& newDhtServerList);

    bool getEnableIPv6() const;
    void setEnableIPv6(bool newValue);

    bool getMakeToxPortable() const;
    void setMakeToxPortable(bool newValue);

    bool getAutostartInTray() const;
    void setAutostartInTray(bool newValue);

    bool getUseTranslations() const;
    void setUseTranslations(bool newValue);

    bool getForceTCP() const;
    void setForceTCP(bool newValue);

    QString getProxyAddr() const;
    void setProxyAddr(const QString& newValue);

    bool getUseProxy() const;
    void setUseProxy(bool newValue);

    int getProxyPort() const;
    void setProxyPort(int newValue);

    bool getEnableLogging() const;
    void setEnableLogging(bool newValue);

    bool getEncryptLogs() const;
    void setEncryptLogs(bool newValue);

    QPixmap getSavedAvatar(const QString& ownerId);
    void saveAvatar(QPixmap& pic, const QString& ownerId);

    QByteArray getAvatarHash(const QString& ownerId);
    void saveAvatarHash(const QByteArray& hash, const QString& ownerId);

    // Assume all widgets have unique names
    // Don't use it to save every single thing you want to save, use it
    // for some general purpose widgets, such as MainWindows or Splitters,
    // which have widget->saveX() and widget->loadX() methods.
    QByteArray getWidgetData(const QString& uniqueName) const;
    void setWidgetData(const QString& uniqueName, const QByteArray& data);

    // Wrappers around getWidgetData() and setWidgetData()
    // Assume widget has a unique objectName set
    template <class T>
    void restoreGeometryState(T* widget) const
    {
        widget->restoreGeometry(getWidgetData(widget->objectName() + "Geometry"));
        widget->restoreState(getWidgetData(widget->objectName() + "State"));
    }
    template <class T>
    void saveGeometryState(const T* widget)
    {
        setWidgetData(widget->objectName() + "Geometry", widget->saveGeometry());
        setWidgetData(widget->objectName() + "State", widget->saveState());
    }

    bool isAnimationEnabled() const;
    void setAnimationEnabled(bool newValue);

    QString getSmileyPack() const;
    void setSmileyPack(const QString &value);

    bool isCurstomEmojiFont() const;
    void setCurstomEmojiFont(bool value);

    QString getEmojiFontFamily() const;
    void setEmojiFontFamily(const QString &value);

    int getEmojiFontPointSize() const;
    void setEmojiFontPointSize(int value);

    // ChatView
    int getFirstColumnHandlePos() const;
    void setFirstColumnHandlePos(const int pos);

    int getSecondColumnHandlePosFromRight() const;
    void setSecondColumnHandlePosFromRight(const int pos);

    const QString &getTimestampFormat() const;
    void setTimestampFormat(const QString &format);

    bool isMinimizeOnCloseEnabled() const;
    void setMinimizeOnClose(bool newValue);

    // Privacy
    bool isTypingNotificationEnabled() const;
    void setTypingNotification(bool enabled);

    bool getUseNativeStyle() const;
    void setUseNativeStyle(bool value);

    QByteArray getWindowGeometry() const;
    void setWindowGeometry(const QByteArray &value);

    QByteArray getWindowState() const;
    void setWindowState(const QByteArray &value);

    QByteArray getSplitterState() const;
    void setSplitterState(const QByteArray &value);

public:
    QList<QString> friendAddresses;
    void save();
    void save(QString path);
    void load();

private:
    Settings();
    Settings(Settings &settings) = delete;
    Settings& operator=(const Settings&) = delete;

    static const QString FILENAME;

    bool loaded;

    bool useCustomDhtList;
    QList<DhtServer> dhtServerList;
    int dhtServerId;
    bool dontShowDhtDialog;

    bool enableIPv6;
    bool useTranslations;
    static bool makeToxPortable;
    bool autostartInTray;

    bool forceTCP;

    bool useProxy;
    QString proxyAddr;
    int proxyPort;

    bool enableLogging;
    bool encryptLogs;

    QHash<QString, QByteArray> widgetSettings;

    // GUI
    bool enableSmoothAnimation;
    QString smileyPack;
    bool customEmojiFont;
    QString emojiFontFamily;
    int     emojiFontPointSize;
    bool minimizeOnClose;
    bool useNativeStyle;
    QByteArray windowGeometry;
    QByteArray windowState;
    QByteArray splitterState;

    // ChatView
    int firstColumnHandlePos;
    int secondColumnHandlePosFromRight;
    QString timestampFormat;

    // Privacy
    bool typingNotification;

signals:
    //void dataChanged();
    void dhtServerListChanged();
    void logStorageOptsChanged();
    void smileyPackChanged();
    void emojiFontChanged();
    void timestampFormatChanged();
};

#endif // SETTINGS_HPP
