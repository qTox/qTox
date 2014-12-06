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

#include "toxsave.h"
#include "widget.h"
#include "src/core.h"
#include "src/misc/settings.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

void toxSaveEventHandler(const QByteArray& eventData)
{
    if (!eventData.endsWith(".tox"))
        return;

    handleToxSave(eventData);
}

void handleToxSave(const QString& path)
{
    Core* core = Core::getInstance();

    while (!core)
    {
        core = Core::getInstance();
        qApp->processEvents();
    }

    while (!core->isReady())
    {
        qApp->processEvents();
    }

    QFileInfo info(path);
    if (!info.exists())
        return;

    QString profile = info.completeBaseName();

    if (info.suffix() != "tox")
    {
        QMessageBox::warning(Widget::getInstance(),
                             QObject::tr("Ignoring non-Tox file", "popup title"),
                             QObject::tr("Warning: you've chosen a file that is not a Tox save file; ignoring.", "popup text"));
        return;
    }

    QString profilePath = QDir(Settings::getSettingsDirPath()).filePath(profile + Core::TOX_EXT);

    if (QFileInfo(profilePath).exists() && !Widget::getInstance()->askQuestion(QObject::tr("Profile already exists", "import confirm title"),
            QObject::tr("A profile named \"%1\" already exists. Do you want to erase it?", "import confirm text").arg(profile)))
        return;

    QFile::copy(path, profilePath);
    // no good way to update the ui from here... maybe we need a Widget:refreshUi() function...
    // such a thing would simplify other code as well I believe
    QMessageBox::information(Widget::getInstance(), QObject::tr("Profile imported"), QObject::tr("%1.tox was successfully imported").arg(profile));
}
