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
        LightGrey,
        White,
    };

    enum Font
    {
        ExtraBig,   // 14px, bold
        Big,        // 12px
        BigBold,    // 12px, bold
        Medium,     // 11px
        MediumBold, // 11px, bold
        Small,      // 10px
        SmallLight  // 10px, light
    };

    static QString getStylesheet(const QString& filename);
    static QColor getColor(ColorPalette entry);
    static QFont getFont(Font font);
private:
    Style();
};

#endif // STYLE_H
