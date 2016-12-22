/*
    Copyright © 2015 by The qTox Project Contributors

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

class Nexus : public QObject
{
    Q_OBJECT
public:
    static Nexus& getInstance();
    static void destroyInstance();
    static QString getSupportedImageFilter();
    static bool tryRemoveFile(const QString& filepath);

    void start();
    void showMainGUI();

public:
    inline Widget* getDesktopGUI() const
    {
        return widget;
    }

    inline Profile* getProfile() const
    {
        return mProfile;
    }

    void setProfile(Profile* mProfile);

public slots:
    void showLogin();
    void showLoginLater();

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
    void updateWindowsArg(QWindow *closedWindow);

    QSignalMapper* windowMapper;
    QActionGroup* windowActions = nullptr;
#endif

private:
    Nexus();
    ~Nexus();

private:
    Profile* mProfile;
    Widget* widget;
    LoginScreen* loginScreen;
};

#endif // NEXUS_H
