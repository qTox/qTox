#include "systemtrayicon.h"
#include <QString>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QFile>
#include <QDebug>
#include "src/persistence/settings.h"

SystemTrayIcon::SystemTrayIcon()
{
    QString desktop = getenv("XDG_CURRENT_DESKTOP");
    if (desktop.isEmpty())
        desktop = getenv("DESKTOP_SESSION");
    desktop = desktop.toLower();
    if (false);
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (desktop == "unity")
    {
        qDebug() << "Using Unity backend";
        gtk_init(nullptr, nullptr);
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
    #ifdef ENABLE_SYSTRAY_GTK_BACKEND
    else if (desktop == "xfce" || desktop.contains("gnome") || desktop == "mate")
    {
        qDebug() << "Using GTK backend";
        backendType = SystrayBackendType::GTK;
        gtk_init(nullptr, nullptr);
        void (*callbackFreeImage)(guchar*, gpointer) =
                [](guchar*, gpointer image)
        {
            delete reinterpret_cast<QImage*>(image);
        };
        QImage* image = new QImage(":/img/icon.png");
        if (image->format() != QImage::Format_RGBA8888_Premultiplied)
        *image = image->convertToFormat(QImage::Format_RGBA8888_Premultiplied);
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(image->bits(), GDK_COLORSPACE_RGB, image->hasAlphaChannel(),
                                 8, image->width(), image->height(),
                                 image->bytesPerLine(), callbackFreeImage, image);
        gtkIcon = gtk_status_icon_new_from_pixbuf(pixbuf);
        gtkMenu = gtk_menu_new();

        void (*callbackTrigger)(GtkStatusIcon*, gpointer) =
                [](GtkStatusIcon*, gpointer data)
        {
            ((SystemTrayIcon*)data)->activated(QSystemTrayIcon::Trigger);
        };
        g_signal_connect(gtkIcon, "activate", G_CALLBACK(callbackTrigger), this);
        void (*callbackButtonClick)(GtkStatusIcon*, GdkEvent*, gpointer) =
                [](GtkStatusIcon*, GdkEvent* event, gpointer data)
        {
            if (event->button.button == 2)
                ((SystemTrayIcon*)data)->activated(QSystemTrayIcon::MiddleClick);
        };
        g_signal_connect(gtkIcon, "button-release-event", G_CALLBACK(callbackButtonClick), this);
    }
    #endif
    #ifdef ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND
    else if (desktop == "kde"
            && getenv("KDE_SESSION_VERSION") == QString("5"))
    {
        qDebug() << "Using Status Notifier backend";
        backendType = SystrayBackendType::StatusNotifier;
        gtk_init(nullptr, nullptr);
        snMenu = gtk_menu_new();
        void (*callbackFreeImage)(guchar*, gpointer) =
                [](guchar*, gpointer image)
        {
            delete reinterpret_cast<QImage*>(image);
        };
        QImage* image = new QImage(":/img/icon.png");
        if (image->format() != QImage::Format_RGBA8888_Premultiplied)
        *image = image->convertToFormat(QImage::Format_RGBA8888_Premultiplied);
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(image->bits(), GDK_COLORSPACE_RGB, image->hasAlphaChannel(),
                                 8, image->width(), image->height(),
                                 image->bytesPerLine(), callbackFreeImage, image);

        statusNotifier = status_notifier_new_from_pixbuf("qtox",
                            STATUS_NOTIFIER_CATEGORY_APPLICATION_STATUS, pixbuf);
        status_notifier_register(statusNotifier);
    }
    #endif
    else if (desktop == "kde"
             && getenv("KDE_SESSION_VERSION") == QString("5"))
    {
        backendType = SystrayBackendType::KDE5;
        qWarning() << "Detected a KDE5 session, but we don't have Status Notifier support. Disabling the systray icon";
    }
    else
    {
        qDebug() << "Using the Qt backend";
        qtIcon = new QSystemTrayIcon;
        backendType = SystrayBackendType::Qt;
        connect(qtIcon, &QSystemTrayIcon::activated, this, &SystemTrayIcon::activated);
    }
}

SystemTrayIcon::~SystemTrayIcon()
{
    qDebug() << "Deleting SystemTrayIcon";
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
    #ifdef ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND
    else if (backendType == SystrayBackendType::StatusNotifier)
    {
        void (*callbackClick)(StatusNotifier*, gint, gint, gpointer) =
                [](StatusNotifier*, gint, gint, gpointer data)
        {
            ((SystemTrayIcon*)data)->activated(QSystemTrayIcon::Trigger);
        };
        g_signal_connect(statusNotifier, "activate", G_CALLBACK(callbackClick), this);
        void (*callbackMiddleClick)(StatusNotifier*, gint, gint, gpointer) =
                [](StatusNotifier*, gint, gint, gpointer data)
        {
            ((SystemTrayIcon*)data)->activated(QSystemTrayIcon::MiddleClick);
        };
        g_signal_connect(statusNotifier, "secondary_activate", G_CALLBACK(callbackMiddleClick), this);

        for (QAction* a : menu->actions())
        {
            QString aText = a->text().replace('&',"");
            GtkWidget* item;
            if (a->isSeparator())
                item = gtk_menu_item_new();
            else if (a->icon().isNull())
                item = gtk_menu_item_new_with_label(aText.toStdString().c_str());
            else
            {
                void (*callbackFreeImage)(guchar*, gpointer) =
                        [](guchar*, gpointer image)
                {
                    delete reinterpret_cast<QImage*>(image);
                };
                QImage* image = new QImage(a->icon().pixmap(64, 64).toImage());
                if (image->format() != QImage::Format_RGBA8888_Premultiplied)
                *image = image->convertToFormat(QImage::Format_RGBA8888_Premultiplied);
                GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(image->bits(), GDK_COLORSPACE_RGB, image->hasAlphaChannel(),
                                         8, image->width(), image->height(),
                                         image->bytesPerLine(), callbackFreeImage, image);
                item = gtk_image_menu_item_new_with_label(aText.toStdString().c_str());
                gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), gtk_image_new_from_pixbuf(pixbuf));
                gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(item),TRUE);
            }
            gtk_menu_shell_append(GTK_MENU_SHELL(snMenu), item);
            void (*callback)(GtkMenu*, gpointer data) = [](GtkMenu*, gpointer a)
            {
                ((QAction*)a)->activate(QAction::Trigger);
            };
            g_signal_connect(item, "activate", G_CALLBACK(callback), a);
            gtk_widget_show(item);
        }
        void (*callbackMenu)(StatusNotifier*, gint, gint, gpointer) =
                [](StatusNotifier*, gint, gint, gpointer data)
        {
            gtk_widget_show_all(((SystemTrayIcon*)data)->snMenu);
            gtk_menu_popup(GTK_MENU(((SystemTrayIcon*)data)->snMenu), 0, 0, 0, 0, 3, gtk_get_current_event_time());
        };
        g_signal_connect(statusNotifier, "context-menu", G_CALLBACK(callbackMenu), this);
    }
    #endif
    #ifdef ENABLE_SYSTRAY_GTK_BACKEND
    else if (backendType == SystrayBackendType::GTK)
    {
        for (QAction* a : menu->actions())
        {
            QString aText = a->text().replace('&',"");
            GtkWidget* item;
            if (a->isSeparator())
                item = gtk_menu_item_new();
            else if (a->icon().isNull())
                item = gtk_menu_item_new_with_label(aText.toStdString().c_str());
            else
            {
                void (*callbackFreeImage)(guchar*, gpointer) =
                        [](guchar*, gpointer image)
                {
                    delete reinterpret_cast<QImage*>(image);
                };
                QImage* image = new QImage(a->icon().pixmap(64, 64).toImage());
                if (image->format() != QImage::Format_RGBA8888_Premultiplied)
                *image = image->convertToFormat(QImage::Format_RGBA8888_Premultiplied);
                GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(image->bits(), GDK_COLORSPACE_RGB, image->hasAlphaChannel(),
                                         8, image->width(), image->height(),
                                         image->bytesPerLine(), callbackFreeImage, image);
                item = gtk_image_menu_item_new_with_label(aText.toStdString().c_str());
                gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), gtk_image_new_from_pixbuf(pixbuf));
                gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(item),TRUE);
            }
            gtk_menu_shell_append(GTK_MENU_SHELL(gtkMenu), item);
            void (*callback)(GtkMenu*, gpointer data) = [](GtkMenu*, gpointer a)
            {
                ((QAction*)a)->activate(QAction::Trigger);
            };
            g_signal_connect(item, "activate", G_CALLBACK(callback), a);
            gtk_widget_show(item);
        }
        void (*callbackMenu)(GtkMenu*, gint, gint, gpointer) =
                [](GtkMenu*, gint, gint, gpointer data)
        {
            gtk_widget_show_all(((SystemTrayIcon*)data)->gtkMenu);
            gtk_menu_popup(GTK_MENU(((SystemTrayIcon*)data)->gtkMenu), 0, 0, 0, 0, 3, gtk_get_current_event_time());
        };
        g_signal_connect(gtkIcon, "popup-menu", G_CALLBACK(callbackMenu), this);
    }
    #endif
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (backendType == SystrayBackendType::Unity)
    {
        for (QAction* a : menu->actions())
        {
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
    else if (backendType == SystrayBackendType::Qt)
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
    #ifdef ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND
    else if (backendType == SystrayBackendType::StatusNotifier)
    {
        if (newState)
            status_notifier_set_status(statusNotifier, STATUS_NOTIFIER_STATUS_ACTIVE);
        else
            status_notifier_set_status(statusNotifier, STATUS_NOTIFIER_STATUS_PASSIVE);
    }
    #endif
    #ifdef ENABLE_SYSTRAY_GTK_BACKEND
    else if (backendType == SystrayBackendType::GTK)
    {
        if (newState)
            gtk_status_icon_set_visible(gtkIcon, true);
        else
            gtk_status_icon_set_visible(gtkIcon, false);
    }
    #endif
    #ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    else if (backendType == SystrayBackendType::Unity)
    {
        if (newState)
            app_indicator_set_status(unityIndicator, APP_INDICATOR_STATUS_ACTIVE);
        else
            app_indicator_set_status(unityIndicator, APP_INDICATOR_STATUS_PASSIVE);
    }
    #endif
    else if (backendType == SystrayBackendType::Qt)
    {
        if (newState)
            qtIcon->show();
        else
            qtIcon->hide();
    }
}

void SystemTrayIcon::setIcon(QIcon &icon)
{
    if (false);
    #ifdef ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND
    else if (backendType == SystrayBackendType::StatusNotifier)
    {
        void (*callbackFreeImage)(guchar*, gpointer) =
                [](guchar*, gpointer image)
        {
            delete reinterpret_cast<QImage*>(image);
        };
        QImage* image = new QImage(icon.pixmap(64, 64).toImage());
        if (image->format() != QImage::Format_RGBA8888_Premultiplied)
        *image = image->convertToFormat(QImage::Format_RGBA8888_Premultiplied);
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(image->bits(), GDK_COLORSPACE_RGB, image->hasAlphaChannel(),
                                 8, image->width(), image->height(),
                                 image->bytesPerLine(), callbackFreeImage, image);
        status_notifier_set_from_pixbuf(statusNotifier, STATUS_NOTIFIER_ICON, pixbuf);
    }
    #endif
    #ifdef ENABLE_SYSTRAY_GTK_BACKEND
    else if (backendType == SystrayBackendType::GTK)
    {
        void (*callbackFreeImage)(guchar*, gpointer) =
                [](guchar*, gpointer image)
        {
            delete reinterpret_cast<QImage*>(image);
        };
        QImage* image = new QImage(icon.pixmap(64, 64).toImage());
        if (image->format() != QImage::Format_RGBA8888_Premultiplied)
        *image = image->convertToFormat(QImage::Format_RGBA8888_Premultiplied);
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(image->bits(), GDK_COLORSPACE_RGB, image->hasAlphaChannel(),
                                 8, image->width(), image->height(),
                                 image->bytesPerLine(), callbackFreeImage, image);
        gtk_status_icon_set_from_pixbuf(gtkIcon, pixbuf);
    }
    #endif
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
    else if (backendType == SystrayBackendType::Qt)
    {
        qtIcon->setIcon(icon);
    }
}
