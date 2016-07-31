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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QFont>
#include <QHash>
#include <QObject>
#include <QPixmap>
#include <QMutex>
#include <QDate>
#include <QNetworkProxy>
#include "src/core/corestructs.h"

class ToxId;
class Profile;
namespace Db { enum class syncType; }

enum ProxyType {ptNone, ptSOCKS5, ptHTTP};

enum StyleType {NONE, WITH_CHARS, WITHOUT_CHARS};

class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings& getInstance();
    static void destroyInstance();
    QString getSettingsDirPath();
    QString getAppDataDirPath();
    QString getAppCacheDirPath();

    void createSettingsDir();
    void createPersonal(QString basename);

    void savePersonal();
    void savePersonal(Profile *profile);

    void loadGlobal();
    void loadPersonal();
    void loadPersonal(Profile *profile);

    struct Request
    {
        QString address;
        QString message;
        bool read;
    };


public slots:
    void saveGlobal();
    void sync();

signals:
    void dhtServerListChanged();
    void smileyPackChanged();
    void emojiFontChanged();

public:
    const QList<DhtServer>& getDhtServerList() const;
    void setDhtServerList(const QList<DhtServer>& newDhtServerList);

    bool getEnableIPv6() const;
    void setEnableIPv6(bool newValue);

    bool getMakeToxPortable() const;
    void setMakeToxPortable(bool newValue);

    bool getAutorun() const;
    void setAutorun(bool newValue);

    bool getAutostartInTray() const;
    void setAutostartInTray(bool newValue);

    bool getCloseToTray() const;
    void setCloseToTray(bool newValue);

    bool getMinimizeToTray() const;
    void setMinimizeToTray(bool newValue);

    bool getLightTrayIcon() const;
    void setLightTrayIcon(bool newValue);

    QString getStyle() const;
    void setStyle(const QString& newValue);

    bool getShowSystemTray() const;
    void setShowSystemTray(const bool& newValue);

    bool getUseEmoticons() const;
    void setUseEmoticons(bool newValue);

    QString getCurrentProfile() const;
    uint32_t getCurrentProfileId() const;
    void setCurrentProfile(QString profile);

    QString getTranslation() const;
    void setTranslation(QString newValue);

    // Toxme
    void deleteToxme();
    void setToxme(QString name, QString server, QString bio, bool priv, QString pass = "");
    QString getToxmeInfo() const;
    void setToxmeInfo(QString info);

    QString getToxmeBio() const;
    void setToxmeBio(QString bio);
    
    bool getToxmePriv() const;
    void setToxmePriv(bool priv);
    
    QString getToxmePass() const;
    void setToxmePass(const QString &pass);
    
    void setAutoSaveEnabled(bool newValue);
    bool getAutoSaveEnabled() const;

    bool getForceTCP() const;
    void setForceTCP(bool newValue);

    QNetworkProxy getProxy() const;

    QString getProxyAddr() const;
    void setProxyAddr(const QString& newValue);

    ProxyType getProxyType() const;
    void setProxyType(int newValue);

    quint16 getProxyPort() const;
    void setProxyPort(quint16 newValue);

    bool getEnableLogging() const;
    void setEnableLogging(bool newValue);

    Db::syncType getDbSyncType() const;
    void setDbSyncType(int newValue);

    int getAutoAwayTime() const;
    void setAutoAwayTime(int newValue);

    bool getCheckUpdates() const;
    void setCheckUpdates(bool newValue);

    bool getShowWindow() const;
    void setShowWindow(bool newValue);

    bool getShowInFront() const;
    void setShowInFront(bool newValue);

    bool getNotifySound() const;
    void setNotifySound(bool newValue);

    bool getBusySound() const;
    void setBusySound(bool newValue);

    bool getGroupAlwaysNotify() const;
    void setGroupAlwaysNotify(bool newValue);

    QString getInDev() const;
    void setInDev(const QString& deviceSpecifier);

    bool getAudioInDevEnabled() const;
    void setAudioInDevEnabled(bool enabled);

    QString getOutDev() const;
    void setOutDev(const QString& deviceSpecifier);

    bool getAudioOutDevEnabled() const;
    void setAudioOutDevEnabled(bool enabled);

    qreal getAudioInGain() const;
    void setAudioInGain(qreal dB);

    int getOutVolume() const;
    void setOutVolume(int volume);

    QString getVideoDev() const;
    void setVideoDev(const QString& deviceSpecifier);

    QRect getScreenRegion() const;
    void setScreenRegion(const QRect &value);

    bool getScreenGrabbed() const;
    void setScreenGrabbed(bool value);

    QRect getCamVideoRes() const;
    void setCamVideoRes(QRect newValue);

    unsigned short getCamVideoFPS() const;
    void setCamVideoFPS(unsigned short newValue);

    bool isAnimationEnabled() const;
    void setAnimationEnabled(bool newValue);

    QString getSmileyPack() const;
    void setSmileyPack(const QString &value);

    int getThemeColor() const;
    void setThemeColor(const int& value);

    StyleType getStylePreference() const;
    void setStylePreference(StyleType newValue);

    bool isCurstomEmojiFont() const;
    void setCurstomEmojiFont(bool value);

    int getEmojiFontPointSize() const;
    void setEmojiFontPointSize(int value);

    QString getContactNote(const ToxId& id) const;
    void setContactNote(const ToxId& id, const QString& note);

    QString getAutoAcceptDir(const ToxId& id) const;
    void setAutoAcceptDir(const ToxId& id, const QString& dir);

    QString getGlobalAutoAcceptDir() const;
    void setGlobalAutoAcceptDir(const QString& dir);

    // ChatView
    const QFont& getChatMessageFont() const;
    void setChatMessageFont(const QFont& font);

    int getFirstColumnHandlePos() const;
    void setFirstColumnHandlePos(const int pos);

    int getSecondColumnHandlePosFromRight() const;
    void setSecondColumnHandlePosFromRight(const int pos);

    const QString& getTimestampFormat() const;
    void setTimestampFormat(const QString& format);

    const QString& getDateFormat() const;
    void setDateFormat(const QString& format);

    bool isMinimizeOnCloseEnabled() const;
    void setMinimizeOnClose(bool newValue);

    bool getStatusChangeNotificationEnabled() const;
    void setStatusChangeNotificationEnabled(bool newValue);

    // Privacy
    bool isTypingNotificationEnabled() const;
    void setTypingNotification(bool enabled);

    // State
    QByteArray getWindowGeometry() const;
    void setWindowGeometry(const QByteArray &value);

    QByteArray getWindowState() const;
    void setWindowState(const QByteArray &value);

    QByteArray getSplitterState() const;
    void setSplitterState(const QByteArray &value);

    QByteArray getDialogGeometry() const;
    void setDialogGeometry(const QByteArray& value);

    QByteArray getDialogSplitterState() const;
    void setDialogSplitterState(const QByteArray &value);

    QByteArray getDialogSettingsGeometry() const;
    void setDialogSettingsGeometry(const QByteArray& value);

    QString getFriendAdress(const QString &publicKey) const;
    void updateFriendAdress(const QString &newAddr);

    QString getFriendAlias(const ToxId &id) const;
    void setFriendAlias(const ToxId &id, const QString &alias);

    int getFriendCircleID(const ToxId &id) const;
    void setFriendCircleID(const ToxId &id, int circleID);

    QDate getFriendActivity(const ToxId &id) const;
    void setFriendActivity(const ToxId &id, const QDate &date);

    void removeFriendSettings(const ToxId &id);

    bool getFauxOfflineMessaging() const;
    void setFauxOfflineMessaging(bool value);

    bool getCompactLayout() const;
    void setCompactLayout(bool compact);

    bool getSeparateWindow() const;
    void setSeparateWindow(bool value);

    bool getDontGroupWindows() const;
    void setDontGroupWindows(bool value);

    bool getGroupchatPosition() const;
    void setGroupchatPosition(bool value);

    bool getAutoLogin() const;
    void setAutoLogin(bool state);

    int getCircleCount() const;
    int addCircle(const QString &name = QString());
    int removeCircle(int id);
    QString getCircleName(int id) const;
    void setCircleName(int id, const QString &name);
    bool getCircleExpanded(int id) const;
    void setCircleExpanded(int id, bool expanded);

    bool addFriendRequest(const QString &friendAddress, const QString &message);
    unsigned int getUnreadFriendRequests() const;
    Request getFriendRequest(int index) const;
    int getFriendRequestSize() const;
    void clearUnreadFriendRequests();
    void removeFriendRequest(int index);
    void readFriendRequest(int index);

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

    static uint32_t makeProfileId(const QString& profile);

private:
    Settings();
    ~Settings();
    Settings(Settings &settings) = delete;
    Settings& operator=(const Settings&) = delete;

private slots:
    void savePersonal(QString profileName, const QString &password);

private:
    bool loaded;

    bool useCustomDhtList;
    QList<DhtServer> dhtServerList;
    int dhtServerId;
    bool dontShowDhtDialog;

    bool autoLogin;
    bool fauxOfflineMessaging;
    bool compactLayout;
    bool groupchatPosition;
    bool separateWindow;
    bool dontGroupWindows;
    bool enableIPv6;
    QString translation;
    bool makeToxPortable;
    bool autostartInTray;
    bool closeToTray;
    bool minimizeToTray;
    bool lightTrayIcon;
    bool useEmoticons;
    bool checkUpdates;
    bool showWindow;
    bool showInFront;
    bool notifySound;
    bool busySound;
    bool groupAlwaysNotify;

    bool forceTCP;

    ProxyType proxyType;
    QString proxyAddr;
    quint16 proxyPort;

    QString currentProfile;
    uint32_t currentProfileId;

    // Toxme Info
    QString toxmeInfo;
    QString toxmeBio;
    bool toxmePriv;
    QString toxmePass;

    bool enableLogging;

    int autoAwayTime;

    QHash<QString, QByteArray> widgetSettings;
    QHash<QString, QString> autoAccept;
    bool autoSaveEnabled;
    QString globalAutoAcceptDir;

    QList<Request> friendRequests;

    // GUI
    QString smileyPack;
    int emojiFontPointSize;
    bool minimizeOnClose;
    QByteArray windowGeometry;
    QByteArray windowState;
    QByteArray splitterState;
    QByteArray dialogGeometry;
    QByteArray dialogSplitterState;
    QByteArray dialogSettingsGeometry;
    QString style;
    bool showSystemTray;

    // ChatView
    QFont chatMessageFont;
    StyleType stylePreference;
    int firstColumnHandlePos;
    int secondColumnHandlePosFromRight;
    QString timestampFormat;
    QString dateFormat;
    bool statusChangeNotificationEnabled;

    // Privacy
    bool typingNotification;
    Db::syncType dbSyncType;

    // Audio
    QString inDev;
    bool audioInDevEnabled;
    qreal audioInGainDecibel;
    QString outDev;
    bool audioOutDevEnabled;
    int outVolume;

    // Video
    QString videoDev;
    QRect camVideoRes;
    QRect screenRegion;
    bool screenGrabbed;
    unsigned short camVideoFPS;

    struct friendProp
    {
        QString alias;
        QString addr;
        QString autoAcceptDir;
        QString note;
        int circleID = -1;
        QDate activity = QDate();
    };

    struct circleProp
    {
        QString name;
        bool expanded;
    };

    QHash<QString, friendProp> friendLst;

    QVector<circleProp> circleLst;

    int themeColor;

    static QMutex bigLock;
    static Settings* settings;
    static const QString globalSettingsFile;
    static QThread* settingsThread;
};

#endif // SETTINGS_HPP
