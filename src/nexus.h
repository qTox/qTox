/*
    Copyright © 2015 by The qTox Project

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


#ifndef NEXUS_H
#define NEXUS_H

#include <QObject>

class Widget;
class AndroidGUI;
class Profile;
class LoginScreen;
class Core;

#ifdef Q_OS_MAC
class QMenuBar;
class QMenu;
class QAction;
class QWindow;
class QActionGroup;
class QSignalMapper;
#endif

/// This class is in charge of connecting various systems together
/// and forwarding signals appropriately to the right objects
/// It is in charge of starting the GUI and the Core
class Nexus : public QObject
{
    Q_OBJECT
public:
    void start(); ///< Sets up invariants and calls showLogin
    void showLogin(); ///< Hides the man GUI, delete the profile, and shows the login screen
    /// Hides the login screen and shows the GUI for the given profile.
    /// Will delete the current GUI, if it exists.
    void showMainGUI();

    static Nexus& getInstance();
    static void destroyInstance();
    static Core* getCore(); ///< Will return 0 if not started
    static Profile* getProfile(); ///< Will return 0 if not started
    static void setProfile(Profile* profile); ///< Delete the current profile, if any, and replaces it
    static AndroidGUI* getAndroidGUI(); ///< Will return 0 if not started
    static Widget* getDesktopGUI(); ///< Will return 0 if not started
    static QString getSupportedImageFilter();
    static bool tryRemoveFile(const QString& filepath); ///< Dangerous way to find out if a path is writable

#ifdef Q_OS_MAC
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
    void updateWindowsArg(QWindow *closedWindow);

    QSignalMapper* windowMapper;
    QActionGroup* windowActions = nullptr;
#endif

private:
    explicit Nexus(QObject *parent = 0);
    ~Nexus();

private:
    Profile* profile;
    Widget* widget;
    AndroidGUI* androidgui;
    LoginScreen* loginScreen;
};

#endif // NEXUS_H
