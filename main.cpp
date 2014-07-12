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
#include "settings.h"
#include <QApplication>
#include <QFontDatabase>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("qTox");
    a.setOrganizationName("Tox");

    // Load translations
    QTranslator translator;
    if (Settings::getInstance().getUseTranslations())
    {
        QString locale = QLocale::system().name().section('_', 0, 0);
        if (translator.load(locale,":translations/"))
            qDebug() << "Loaded translation "+locale;
        else
            qDebug() << "Error loading translation "+locale;
        a.installTranslator(&translator);
    }

    // Install Unicode 6.1 supporting font
    QFontDatabase::addApplicationFont("://DejaVuSans.ttf");

    Widget* w = Widget::getInstance();
    w->show();

    int errorcode = a.exec();

    delete w;

    return errorcode;
}

/** TODO
 * ">using a dedicated tool to maintain a TODO list" edition
 *
 * QRC FILES DO YOU EVEN INTO THEM ? Fix it soon for packaging and Urras.
 * Most cameras use YUYV, implement YUYV -> YUV240
 * Sending large files (~380MB) "restarts" after ~10MB. Goes back to 0%, consumes twice as much ram (reloads the file?)
 * => Don't load the whole file at once, load small chunks (25MB?) when needed, then free them and load the next
 * Sort the friend list by status, online first then busy then offline
 * Don't do anything if a friend is disconnected, don't print to the chat
 * Changing online/away/busy/offline by clicking the bubble
 * /me action messages
 * Popup windows for adding friends and changing settings
 * And logging of the chat
 * Show the picture's size between name and size after transfer completion if it's a pic
 * Adjust all status icons to match the mockup, including scooting the friendslist ones to the left and making the user one the same size
 * Sidepanel (friendlist) should be resizeable
 * An extra side panel for groupchats, like Venom does (?)
 *
 * In the file transfer widget:
 * >There is more padding on the left side compared to the right.
 * >Maybe put the file size should be in the same row as the name.
 * >Right-align the ETA.
 *
 */

/** NAMES :
Botox
Ricin
Anthrax
Sarin
Cyanide
Polonium
Mercury
Arsenic
qTox
plague
Britney
Nightshade
Belladonna
toxer
GoyIM
 */
