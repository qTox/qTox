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


#include "settingsDir.h"
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>

const QString FILENAME = "settings.ini";

QString getSettingsDirPath()
{
    if (isToxPortableEnabled())
        return ".";

#ifdef Q_OS_WIN
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "AppData" + QDir::separator() + "Roaming" + QDir::separator() + "tox");
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "tox");
#endif
}

bool isToxPortableEnabled()
{
    QFile portableSettings(FILENAME);
    if (portableSettings.exists())
    {
        QSettings ps(FILENAME, QSettings::IniFormat);
        ps.beginGroup("General");
        return ps.value("makeToxPortable", false).toBool();
    }
    else
    {
        return false;
    }
}
