/*
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

#include "toxsave.h"
#include "gui.h"
#include "src/core/core.h"
#include "src/misc/settings.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

bool toxSaveEventHandler(const QByteArray& eventData)
{
    if (!eventData.endsWith(".tox"))
        return false;

    handleToxSave(eventData);
    return true;
}

bool handleToxSave(const QString& path)
{
    Core* core = Core::getInstance();

    while (!core)
    {
        core = Core::getInstance();
        qApp->processEvents();
    }

    while (!core->isReady())
        qApp->processEvents();

    QFileInfo info(path);
    if (!info.exists())
        return false;

    QString profile = info.completeBaseName();

    if (info.suffix() != "tox")
    {
        GUI::showWarning(QObject::tr("Ignoring non-Tox file", "popup title"),
                         QObject::tr("Warning: you've chosen a file that is not a Tox save file; ignoring.", "popup text"));
        return false;
    }

    QString profilePath = QDir(Settings::getSettingsDirPath()).filePath(profile + Core::TOX_EXT);

    if (QFileInfo(profilePath).exists() && !GUI::askQuestion(QObject::tr("Profile already exists", "import confirm title"),
            QObject::tr("A profile named \"%1\" already exists. Do you want to erase it?", "import confirm text").arg(profile)))
        return false;

    QFile::copy(path, profilePath);
    // no good way to update the ui from here... maybe we need a Widget:refreshUi() function...
    // such a thing would simplify other code as well I believe
    GUI::showInfo(QObject::tr("Profile imported"), QObject::tr("%1.tox was successfully imported").arg(profile));
    return true;
}
