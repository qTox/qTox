/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#pragma once

#include <QColor>
#include <QFont>
#include <QMap>
#include <QObject>

class QString;
class QWidget;
class Settings;

class Style : public QObject
{
Q_OBJECT
public:
    enum class ColorPalette
    {
        TransferGood,
        TransferWait,
        TransferBad,
        TransferMiddle,
        MainText,
        NameActive,
        StatusActive,
        GroundExtra,
        GroundBase,
        Orange,
        Yellow,
        ThemeDark,
        ThemeMediumDark,
        ThemeMedium,
        ThemeLight,
        Action,
        Link,
        SearchHighlighted,
        SelectText
    };

    enum class Font
    {
        ExtraBig,
        Big,
        BigBold,
        Medium,
        MediumBold,
        Small,
        SmallLight
    };

    enum class MainTheme
    {
        Light,
        Dark
    };

    static QStringList getThemeColorNames();
    static QString getThemeFolder(Settings& settings);
    static QString getThemeName();
    static QFont getFont(Font font);
    static void repolish(QWidget* w);
    void applyTheme();
    static QPixmap scaleSvgImage(const QString& path, uint32_t width, uint32_t height);

    Style() = default;
    const QString getStylesheet(const QString& filename, Settings& settings, const QFont& baseFont = QFont());
    const QString getImagePath(const QString& filename, Settings& settings);
    QColor getColor(ColorPalette entry);
    const QString resolve(const QString& filename, Settings& settings, const QFont& baseFont = QFont());
    void setThemeColor(Settings& settings, int color);
    void setThemeColor(const QColor& color);
    void initPalette(Settings& settings);
    void initDictColor();
    static QString getThemePath(Settings& settings);

signals:
    void themeReload();

private:
    QMap<ColorPalette, QColor> palette;
    QMap<QString, QString> dictColor;
    QMap<QString, QString> dictFont;
    QMap<QString, QString> dictTheme;
    // stylesheet filename, font -> stylesheet
    // QString implicit sharing deduplicates stylesheets rather than constructing a new one each time
    std::map<std::pair<const QString, const QFont>, const QString> stylesheetsCache;
    QStringList existingImagesCache;
};
