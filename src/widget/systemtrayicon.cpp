#include "systemtrayicon.h"
#include <QString>
#include <QSystemTrayIcon>
#include <QDebug>

SystemTrayIcon::SystemTrayIcon()
{
    QString desktop = getenv("XDG_CURRENT_DESKTOP");
    if (false);
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (desktop.toLower() == "unity")
    {
        backendType = SystrayBackendType::Unity;
        unityMenu = gtk_menu_new();
    }
    #endif
    else
    {
        qtIcon = new QSystemTrayIcon;
        backendType = SystrayBackendType::Qt;
    }
}

void SystemTrayIcon::setContextMenu(QMenu* menu)
{
    if (false);
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (backendType == SystrayBackendType::Unity)
    {
        qCritical() << "SystemTrayIcon::setContextMenu: Not implemented with Unity backend";
    }
    #endif
    else
    {
        qtIcon->setContextMenu(menu);
    }
}

void SystemTrayIcon::show()
{
    setVisible(true);
}

void SystemTrayIcon::hide()
{
    setVisible(false);
}

void SystemTrayIcon::setVisible(bool newState)
{
    if (false);
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (backendType == SystrayBackendType::Unity)
    {
        qCritical() << "SystemTrayIcon::setVisible: Not implemented with Unity backend";
    }
    #endif
    else
    {
        if (newState)
            qtIcon->show();
        else
            qtIcon->hide();
    }
}

void SystemTrayIcon::setIcon(QIcon &&icon)
{
    if (false);
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (backendType == SystrayBackendType::Unity)
    {
        qCritical() << "SystemTrayIcon::setIcon: Not implemented with Unity backend";
    }
    #endif
    else
    {
        qtIcon->setIcon(icon);
    }
}
