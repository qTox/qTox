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

#ifndef STYLE_H
#define STYLE_H

#include <QColor>
#include <QFont>

class QString;
class QWidget;

class Style
{
public:
    enum ColorPalette
    {
        Green,
        Yellow,
        Red,
        Black,
        DarkGrey,
        MediumGrey,
        MediumGreyLight,
        LightGrey,
        White,
        Orange,
    };

    enum Font
    {
        ExtraBig,   // [SystemDefault + 2]px, bold
        Big,        // [SystemDefault    ]px
        BigBold,    // [SystemDefault    ]px, bold
        Medium,     // [SystemDefault - 1]px
        MediumBold, // [SystemDefault - 1]px, bold
        Small,      // [SystemDefault - 2]px
        SmallLight  // [SystemDefault - 2]px, light
    };

    static QString getStylesheet(const QString& filename);
    static QColor getColor(ColorPalette entry);
    static QFont getFont(Font font);
    static QString resolve(QString qss);
    static void repolish(QWidget* w);

private:
    Style();
};

#endif // STYLE_H
