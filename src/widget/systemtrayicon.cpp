#include "systemtrayicon.h"
#include <QString>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QFile>
#include <QDebug>
#include "src/misc/settings.h"

SystemTrayIcon::SystemTrayIcon()
{
    QString desktop = getenv("XDG_CURRENT_DESKTOP");
    if (false);
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (desktop.toLower() == "unity")
    {
        QString settingsDir = Settings::getSettingsDirPath();
        QFile iconFile(settingsDir+"/icon.png");
        if (iconFile.open(QIODevice::Truncate | QIODevice::WriteOnly))
        {
            QFile resIconFile(":/img/icon.png");
            if (resIconFile.open(QIODevice::ReadOnly))
                iconFile.write(resIconFile.readAll());
            resIconFile.close();
            iconFile.close();
        }
        backendType = SystrayBackendType::Unity;
        unityMenu = gtk_menu_new();
        unityIndicator = app_indicator_new_with_path(
            "qTox",
            "icon",
            APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
            settingsDir.toStdString().c_str()
        );
        app_indicator_set_menu(unityIndicator, GTK_MENU(unityMenu));
    }
    #endif
    else
    {
        qtIcon = new QSystemTrayIcon;
        backendType = SystrayBackendType::Qt;
        connect(qtIcon, &QSystemTrayIcon::activated, this, &SystemTrayIcon::activated);
    }
}

QString SystemTrayIcon::extractIconToFile(QIcon icon, QString name)
{
    QString iconPath;
    (void) icon;
    (void) name;
#ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    iconPath = Settings::getSettingsDirPath()+"/"+name+".png";
    QSize iconSize = icon.actualSize(QSize{64,64});
    icon.pixmap(iconSize).save(iconPath);
#endif
    return iconPath;
}

void SystemTrayIcon::setContextMenu(QMenu* menu)
{
    if (false);
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (backendType == SystrayBackendType::Unity)
    {
        for (QAction* a : menu->actions())
        {
            gtk_image_menu_item_new();
            QString aText = a->text().replace('&',"");
            GtkWidget* item;
            if (a->isSeparator())
                item = gtk_menu_item_new();
            else if (a->icon().isNull())
                item = gtk_menu_item_new_with_label(aText.toStdString().c_str());
            else
            {
                QString iconPath = extractIconToFile(a->icon(),"iconmenu"+a->icon().name());
                GtkWidget* image = gtk_image_new_from_file(iconPath.toStdString().c_str());
                item = gtk_image_menu_item_new_with_label(aText.toStdString().c_str());
                gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
                gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(item),TRUE);
            }
            gtk_menu_shell_append(GTK_MENU_SHELL(unityMenu), item);
            void (*callback)(GtkMenu*, gpointer data) = [](GtkMenu*, gpointer a)
            {
                ((QAction*)a)->activate(QAction::Trigger);
            };
            g_signal_connect(item, "activate", G_CALLBACK(callback), a);
            gtk_widget_show(item);
        }
        app_indicator_set_menu(unityIndicator, GTK_MENU(unityMenu));
        DbusmenuServer *menuServer;
        DbusmenuMenuitem *rootMenuItem;
        g_object_get(unityIndicator, "dbus-menu-server", &menuServer, NULL);
        g_object_get(menuServer, "root-node", &rootMenuItem, NULL);
        void (*callback)(DbusmenuMenuitem *, gpointer) =
                [](DbusmenuMenuitem *, gpointer data)
        {
            ((SystemTrayIcon*)data)->activated(QSystemTrayIcon::Unknown);
        };
        g_signal_connect(rootMenuItem, "about-to-show", G_CALLBACK(callback), this);
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
        if (newState)
            app_indicator_set_status(unityIndicator, APP_INDICATOR_STATUS_ACTIVE);
        else
            app_indicator_set_status(unityIndicator, APP_INDICATOR_STATUS_PASSIVE);
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
        // Alternate file names or appindicator will not reload the icon
        if (app_indicator_get_icon(unityIndicator) == QString("icon2"))
        {
            extractIconToFile(icon,"icon");
            app_indicator_set_icon_full(unityIndicator, "icon","qtox");
        }
        else
        {
            extractIconToFile(icon,"icon2");
            app_indicator_set_icon_full(unityIndicator, "icon2","qtox");
        }
    }
    #endif
    else
    {
        qtIcon->setIcon(icon);
    }
}
