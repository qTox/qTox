/*
    Copyright © 2014-2015 by The qTox Project Contributors

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
#include <memory>

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
    static std::shared_ptr<QString> getStylesheet(const QString& filename, QFont baseFont = QFont());
    static QColor getColor(ColorPalette entry);
    static QFont getFont(Font font);
    static std::shared_ptr<QString> factory(QString qss, QFont baseFont);
    static std::shared_ptr<QString> resolve(const QString& filename, QFont baseFont);
    static void repolish(QWidget* w);
    static void setThemeColor(int color);
    static void setThemeColor(const QColor& color);
    static void applyTheme();
    static QPixmap scaleSvgImage(const QString& path, uint32_t width, uint32_t height);

    static QList<QColor> themeColorColors;
    static std::map<std::pair<QString, QFont>, std::weak_ptr<QString>> stylesheetsCache;

signals:
    void themeChanged();

private:
    Style();
};

#endif // STYLE_H
