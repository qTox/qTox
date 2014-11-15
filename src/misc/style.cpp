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
#include <QMap>
#include <QRegularExpression>
#include <QWidget>
#include <QStyle>
#include <QFontInfo>

// helper functions
QFont appFont(int pixelSize, int weight)
{
    QFont font;
    font.setPixelSize(pixelSize);
    font.setWeight(weight);
    return font;
}

QString qssifyFont(QFont font)
{
    return QString("%1 %2px \"%3\"")
            .arg(font.weight()*8)
            .arg(font.pixelSize())
            .arg(font.family());
}

QString Style::getStylesheet(const QString &filename)
{
    if (!Settings::getInstance().getUseNativeStyle())
    {
        QFile file(filename);
        if (file.open(QFile::ReadOnly | QFile::Text))
            return resolve(file.readAll());
        else
            qWarning() << "Style: Stylesheet " << filename << " not found";
    }

    return QString();
}

QColor Style::getColor(Style::ColorPalette entry)
{
    // colors as defined in
    // https://github.com/ItsDuke/Tox-UI/blob/master/UI%20GUIDELINES.md
    static QColor palette[] = {
        QColor("#6bc260"),
        QColor("#cebf44"),
        QColor("#c84e4e"),
        QColor("#000000"),
        QColor("#1c1c1c"),
        QColor("#414141"),
        QColor("#414141").lighter(120),
        QColor("#d1d1d1"),
        QColor("#ffffff"),
        QColor("#ff7700"),

        // Theme colors
        QColor("#1c1c1c"),
        QColor("#2a2a2a"),
        QColor("#414141"),
        QColor("#4e4e4e"),
    };

    return palette[entry];
}

QFont Style::getFont(Style::Font font)
{
    // fonts as defined in
    // https://github.com/ItsDuke/Tox-UI/blob/master/UI%20GUIDELINES.md

    static int defSize = QFontInfo(QFont()).pixelSize();

    static QFont fonts[] = {
        appFont(defSize + 2, QFont::Bold),      // extra big
        appFont(defSize    , QFont::Normal),    // big
        appFont(defSize    , QFont::Bold),      // big bold
        appFont(defSize - 1, QFont::Normal),    // medium
        appFont(defSize - 1, QFont::Bold),      // medium bold
        appFont(defSize - 2, QFont::Normal),    // small
        appFont(defSize - 2, QFont::Light),     // small light
    };

    return fonts[font];
}

QString Style::resolve(QString qss)
{
    static QMap<QString, QString> dict = {
        // colors
        {"@green", getColor(Green).name()},
        {"@yellow", getColor(Yellow).name()},
        {"@red", getColor(Red).name()},
        {"@black", getColor(Black).name()},
        {"@darkGrey", getColor(DarkGrey).name()},
        {"@mediumGrey", getColor(MediumGrey).name()},
        {"@mediumGreyLight", getColor(MediumGreyLight).name()},
        {"@lightGrey", getColor(LightGrey).name()},
        {"@white", getColor(White).name()},
        {"@orange", getColor(Orange).name()},
        {"@themeDark", getColor(ThemeDark).name()},
        {"@themeMediumDark", getColor(ThemeMediumDark).name()},
        {"@themeMedium", getColor(ThemeMedium).name()},
        {"@themeLight", getColor(ThemeLight).name()},

        // fonts
        {"@extraBig", qssifyFont(getFont(ExtraBig))},
        {"@big", qssifyFont(getFont(Big))},
        {"@bigBold", qssifyFont(getFont(BigBold))},
        {"@medium", qssifyFont(getFont(Medium))},
        {"@mediumBold", qssifyFont(getFont(MediumBold))},
        {"@small", qssifyFont(getFont(Small))},
        {"@smallLight", qssifyFont(getFont(SmallLight))},
    };

    for (const QString& key : dict.keys())
    {
        qss.replace(QRegularExpression(QString("%1\\b").arg(key)), dict[key]);
    }

    return qss;
}

void Style::repolish(QWidget *w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);

    for (QObject* o : w->children())
    {
        QWidget* c = qobject_cast<QWidget*>(o);
        if (c)
        {
            c->style()->unpolish(c);
            c->style()->polish(c);
        }
    }
}
