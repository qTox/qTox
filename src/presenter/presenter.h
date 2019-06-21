//
// Created by kriby on 6/17/19.
//

#ifndef QTOX_PRESENTER_H
#define QTOX_PRESENTER_H


#include <QObject>
#include <QtCore/QVector>
#include <src/core/toxpk.h>

#ifdef Q_OS_MAC
#include <QActionGroup>
#include <QMenuBar>
#include <QSignalMapper>
#include <QWindow>
#endif

class Widget;
class Profile;
class Core;
/**
 * The Presenter class should be responsible for linking view interface elements with appropriate
 * model altering methods, and linking model changes to appropriate slots in views. It should not
 * contain model information, nor should it instantiate views (eg. mainGUI, audioNotifications..)
 */
class Presenter : public QObject
{
    Q_OBJECT

#ifdef Q_OS_MAC
public:
    QMenuBar* globalMenuBar;
    QMenu* viewMenu;
    QMenu* windowMenu;
    QAction* minimizeAction;
    QAction* fullscreenAction;
    QAction* frontAction;
    QMenu* dockMenu;
    void setupMacEnvironment();

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

    QSignalMapper* windowMapper;
    QActionGroup* windowActions = nullptr;
#endif

public:
    Presenter();

    void setupEnvironment();

public slots:
};


#endif // QTOX_PRESENTER_H
