/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "widget/widget.h"
#include "misc/settings.h"
#include <QApplication>
#include <QFontDatabase>
#include <QSystemTrayIcon>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("qTox");
    a.setOrganizationName("Tox");

    // Windows platform plugins DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    a.addLibraryPath("platforms");

    // Install Unicode 6.1 supporting font
    QFontDatabase::addApplicationFont("://DejaVuSans.ttf");

    Widget* w = Widget::getInstance();
    if (QSystemTrayIcon::isSystemTrayAvailable() == false)
    {
        qWarning() << "No system tray detected!";
        w->show();
        }
        else
        {
            QSystemTrayIcon *icon = new QSystemTrayIcon(w);
            QObject::connect(icon,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                            w,
                            SLOT(onIconClick()));
                            icon->setIcon(w->windowIcon());
            icon->show();
            if(Settings::getInstance().getAutostartInTray() == false)
                w->show();
    }


    int errorcode = a.exec();

    delete w;

    return errorcode;
}
