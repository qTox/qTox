/*
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
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
        ThemeDark,
        ThemeMediumDark,
        ThemeMedium,
        ThemeLight,
    };

    enum Font
    {
        ExtraBig,
        Big,
        BigBold,
        Medium,
        MediumBold,
        Small,
        SmallLight
    };

    static QStringList getThemeColorNames();
    static QString getStylesheet(const QString& filename, const QFont& baseFont = QFont());
    static QColor getColor(ColorPalette entry);
    static QFont getFont(Font font);
    static QString resolve(QString qss, const QFont& baseFont = QFont());
    static void repolish(QWidget* w);
    static void setThemeColor(int color);
    static void setThemeColor(const QColor &color);
    static void applyTheme();
    static QPixmap scaleSvgImage(const QString& path, uint32_t width, uint32_t height);

    static QList<QColor> themeColorColors;

signals:
    void themeChanged();

private:
    Style();
};

#endif // STYLE_H
