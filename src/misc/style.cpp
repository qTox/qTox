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

#include "src/widget/widget.h"
#include "ui_mainwindow.h"
#include "src/widget/genericchatroomwidget.h"

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

static QMap<QString, QString> dict;

QStringList Style::themeColorNames = {QObject::tr("Default"), QObject::tr("Blue"), QObject::tr("Olive"), QObject::tr("Red"), QObject::tr("Violet")};
QList<QColor> Style::themeColorColors = {QColor(), QColor("#004aa4"), QColor("#97ba00"), QColor("#c23716"), QColor("#4617b5")};


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
    if (dict.isEmpty())
    {
        dict = {
            // colors
            {"@green", Style::getColor(Style::Green).name()},
            {"@yellow", Style::getColor(Style::Yellow).name()},
            {"@red", Style::getColor(Style::Red).name()},
            {"@black", Style::getColor(Style::Black).name()},
            {"@darkGrey", Style::getColor(Style::DarkGrey).name()},
            {"@mediumGrey", Style::getColor(Style::MediumGrey).name()},
            {"@mediumGreyLight", Style::getColor(Style::MediumGreyLight).name()},
            {"@lightGrey", Style::getColor(Style::LightGrey).name()},
            {"@white", Style::getColor(Style::White).name()},
            {"@orange", Style::getColor(Style::Orange).name()},
            {"@themeDark", Style::getColor(Style::ThemeDark).name()},
            {"@themeMediumDark", Style::getColor(Style::ThemeMediumDark).name()},
            {"@themeMedium", Style::getColor(Style::ThemeMedium).name()},
            {"@themeLight", Style::getColor(Style::ThemeLight).name()},

            // fonts
            {"@extraBig", qssifyFont(Style::getFont(Style::ExtraBig))},
            {"@big", qssifyFont(Style::getFont(Style::Big))},
            {"@bigBold", qssifyFont(Style::getFont(Style::BigBold))},
            {"@medium", qssifyFont(Style::getFont(Style::Medium))},
            {"@mediumBold", qssifyFont(Style::getFont(Style::MediumBold))},
            {"@small", qssifyFont(Style::getFont(Style::Small))},
            {"@smallLight", qssifyFont(Style::getFont(Style::SmallLight))},
        };
    }

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

void Style::setThemeColor(int color)
{
    if (color < 0 || color >= themeColorColors.size())
        setThemeColor(QColor());
    else
        setThemeColor(themeColorColors[color]);
}

void Style::setThemeColor(QColor color)
{
    if (!color.isValid())
    {
        // Reset to default
        palette[ThemeDark] = QColor("#1c1c1c");
        palette[ThemeMediumDark] = QColor("#2a2a2a");
        palette[ThemeMedium] = QColor("#414141");
        palette[ThemeLight] = QColor("#4e4e4e");
    }
    else
    {
        palette[ThemeDark] = color.darker(155);
        palette[ThemeMediumDark] = color.darker(135);
        palette[ThemeMedium] = color.darker(120);
        palette[ThemeLight] = color.lighter(110);
    }

    dict["@themeDark"] = getColor(ThemeDark).name();
    dict["@themeMediumDark"] = getColor(ThemeMediumDark).name();
    dict["@themeMedium"] = getColor(ThemeMedium).name();
    dict["@themeLight"] = getColor(ThemeLight).name();

    applyTheme();
}

void Style::applyTheme()
{
    Widget::getInstance()->reloadTheme();
}
