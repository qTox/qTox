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

#include "style.h"
#include "settings.h"

#include <QFile>
#include <QDebug>
#include <QVector>

QString Style::getStylesheet(const QString &filename)
{
    if (!Settings::getInstance().getUseNativeStyle())
    {
        QFile file(filename);
        if (file.open(QFile::ReadOnly | QFile::Text))
            return file.readAll();
        else
            qWarning() << "Stylesheet " << filename << " not found";
    }

    return QString();
}

QColor Style::getColor(Style::ColorPalette entry)
{
    static QColor palette[] = {
        QColor("#6bc260"),
        QColor("#cebf44"),
        QColor("#c84e4e"),
        QColor("#000000"),
        QColor("#1c1c1c"),
        QColor("#414141"),
        QColor("#d1d1d1"),
        QColor("#ffffff"),
    };

    return palette[entry];
}

QFont appFont(int pixelSize, int weight) {
    auto font = QFont();
    font.setPixelSize(pixelSize);
    font.setWeight(weight);
    return font;
}

QFont Style::getFont(Style::Font font)
{
    static QFont fonts[] = {
        appFont(14, QFont::Bold),
        appFont(12, QFont::Normal),
        appFont(12, QFont::Bold),
        appFont(11, QFont::Normal),
        appFont(11, QFont::Bold),
        appFont(10, QFont::Normal),
        appFont(10, QFont::Light),
    };

    return fonts[font];
}
