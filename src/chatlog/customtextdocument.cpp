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

#include "customtextdocument.h"
#include "../misc/settings.h"
#include "../misc/smileypack.h"
#include "../misc/style.h"

#include <QIcon>
#include <QDebug>
#include <QUrl>

CustomTextDocument::CustomTextDocument(QObject *parent)
    : QTextDocument(parent)
{
    static QString css = Style::getStylesheet(":ui/chatArea/innerStyle.css");

    setDefaultStyleSheet(css);
    setUndoRedoEnabled(false);
    setUseDesignMetrics(false);
}

QVariant CustomTextDocument::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource && name.scheme() == "key")
    {
        QSize size = QSize(Settings::getInstance().getEmojiFontPointSize(),Settings::getInstance().getEmojiFontPointSize());
        QString fileName = QUrl::fromPercentEncoding(name.toEncoded()).mid(4).toHtmlEscaped();

        return SmileyPack::getInstance().getAsIcon(fileName).pixmap(size);
    }

    return QTextDocument::loadResource(type, name);
}
