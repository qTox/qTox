/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "audio/iaudiocontrol.h"

#include <QObject>

#include <memory>

class Widget;
class Profile;
class Settings;
class LoginScreen;
class Core;
class QCommandLineParser;
class CameraSource;
class Style;
class IMessageBoxManager;

#ifdef Q_OS_MAC
class QMenuBar;
class QMenu;
class QAction;
class QWindow;
class QActionGroup;
class QSignalMapper;
#endif

class Nexus : public QObject
{
    Q_OBJECT
public:
    void start();
    void showMainGUI();
    void setSettings(Settings* settings_);
    void setMessageBoxManager(IMessageBoxManager* messageBoxManager);
    void setParser(QCommandLineParser* parser_);
    static Nexus& getInstance();
    static void destroyInstance();
    Profile* getProfile();
    static Widget* getDesktopGUI();
    static CameraSource& getCameraSource();


#ifdef Q_OS_MAC
public:
    QMenuBar* globalMenuBar;
    QMenu* viewMenu;
    QMenu* windowMenu;
    QAction* minimizeAction;
    QAction* fullscreenAction;
    QAction* frontAction;
    QMenu* dockMenu;

public slots:
    void retranslateUi();
    void onWindowStateChanged(Qt::WindowStates state);
    void updateWindows();
    void updateWindowsClosed();
    void updateWindowsStates();
    void onOpenWindow(QObject* object);
    void toggleFullscreen();
    void bringAllToFront();

private:
    void updateWindowsArg(QWindow* closedWindow);

    QActionGroup* windowActions = nullptr;
#endif
signals:
    void currentProfileChanged(Profile* Profile);
    void profileLoaded();
    void profileLoadFailed();
    void saveGlobal();

public slots:
    void onCreateNewProfile(const QString& name, const QString& pass);
    void onLoadProfile(const QString& name, const QString& pass);
    int showLogin(const QString& profileName = QString());
    void bootstrapWithProfile(Profile* p);

private:
    explicit Nexus(QObject* parent = nullptr);
    void connectLoginScreen(const LoginScreen& loginScreen);
    void setProfile(Profile* p);
    ~Nexus();

private:
    Profile* profile;
    Settings* settings;
    Widget* widget;
    std::unique_ptr<IAudioControl> audioControl;
    QCommandLineParser* parser = nullptr;
    std::unique_ptr<CameraSource> cameraSource;
    std::unique_ptr<Style> style;
    IMessageBoxManager* messageBoxManager = nullptr;
};
