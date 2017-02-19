/*
    Copyright © 2017 by The qTox Project Contributors

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

#include "textformatter.h"

#include <QMap>
#include <QPair>
#include <QRegularExpression>
#include <QVector>

enum TextStyle {
    BOLD = 0,
    ITALIC,
    UNDERLINE,
    STRIKE,
    CODE
};

static const QString COMMON_PATTERN = QStringLiteral("(?<=^|[^%1<])"
                                                     "[%1]{%3}"
                                                     "(?![%1 \\n])"
                                                     ".+?"
                                                     "(?<![%1< \\n])"
                                                     "[%1]{%3}"
                                                     "(?=$|[^%1])");

static const QString MULTILINE_CODE = QStringLiteral("(?<=^|[^`])"
                                                     "```"
                                                     "(?!`)"
                                                     "(.|\\n)+"
                                                     "(?<!`)"
                                                     "```"
                                                     "(?=$|[^`])");

// Items in vector associated with TextStyle values respectively. Do NOT change this order
static const QVector<QString> fontStylePatterns
{
    QStringLiteral("<b>%1</b>"),
    QStringLiteral("<i>%1</i>"),
    QStringLiteral("<u>%1</u>"),
    QStringLiteral("<s>%1</s>"),
    QStringLiteral("<font color=#595959><code>%1</code></font>")
};

// Unfortunately, can't use simple QMap because ordered applying of styles is required
static const QVector<QPair<QRegularExpression, QString>> textPatternStyle
{
    { QRegularExpression(COMMON_PATTERN.arg("*", "1")), fontStylePatterns[BOLD] },
    { QRegularExpression(COMMON_PATTERN.arg("/", "1")), fontStylePatterns[ITALIC] },
    { QRegularExpression(COMMON_PATTERN.arg("_", "1")), fontStylePatterns[UNDERLINE] },
    { QRegularExpression(COMMON_PATTERN.arg("~", "1")), fontStylePatterns[STRIKE] },
    { QRegularExpression(COMMON_PATTERN.arg("`", "1")), fontStylePatterns[CODE] },
    { QRegularExpression(COMMON_PATTERN.arg("*", "2")), fontStylePatterns[BOLD] },
    { QRegularExpression(COMMON_PATTERN.arg("/", "2")), fontStylePatterns[ITALIC] },
    { QRegularExpression(COMMON_PATTERN.arg("_", "2")), fontStylePatterns[UNDERLINE] },
    { QRegularExpression(COMMON_PATTERN.arg("~", "2")), fontStylePatterns[STRIKE] },
    { QRegularExpression(MULTILINE_CODE), fontStylePatterns[CODE] }
};

TextFormatter::TextFormatter(const QString &str)
    : sourceString(str)
{
}

/**
 * @brief TextFormatter::patternEscapeSignsCount Counts equal symbols at the beginning of the string
 * @param str Source string
 * @return Amount of equal symbols at the beginning of the string
 */
int TextFormatter::patternEscapeSignsCount(const QString &str)
{
    QChar escapeSign = str.at(0);
    int result = 0;
    int length = str.length();
    while (result < length && str[result] == escapeSign)
    {
        ++result;
    }
    return result;
}

/**
 * @brief TextFormatter::isTagIntersection Checks HTML tags intersection while applying styles to the message text
 * @param str Checking string
 * @return True, if tag intersection detected
 */
bool TextFormatter::isTagIntersection(const QString str)
{
    const QRegularExpression TAG_PATTERN("(?<=<)/?[a-zA-Z0-9]+(?=>)");

    int openingTagCount = 0;
    int closingTagCount = 0;

    QRegularExpressionMatchIterator iter = TAG_PATTERN.globalMatch(str);
    while (iter.hasNext())
    {
        iter.next().captured()[0] == '/'
                ? ++closingTagCount
                : ++openingTagCount;
    }
    return openingTagCount != closingTagCount;
}

/**
 * @brief TextFormatter::applyHtmlFontStyling Applies styles to the font of text that was passed to the constructor
 * @param dontShowFormattingSymbols True, if it does not suppose to include formatting symbols into resulting string
 * @return Source text with styled font
 */
QString TextFormatter::applyHtmlFontStyling(bool dontShowFormattingSymbols)
{
    QString out = sourceString;
    int multiplier = dontShowFormattingSymbols ? 1 : 0;

    for (QPair<QRegularExpression, QString> pair : textPatternStyle)
    {
        QRegularExpressionMatchIterator matchesIterator = pair.first.globalMatch(out);
        int insertedTagSymbolsCount = 0;

        while (matchesIterator.hasNext())
        {
            QRegularExpressionMatch match = matchesIterator.next();
            if (isTagIntersection(match.captured()))
            {
                continue;
            }

            int capturedStart = match.capturedStart() + insertedTagSymbolsCount;
            int capturedLength = match.capturedLength();

            int choppingSignsCount = patternEscapeSignsCount(out.mid(capturedStart, capturedLength)) * multiplier;
            int textStart = capturedStart + choppingSignsCount * multiplier;
            int textLength = capturedLength - 2 * choppingSignsCount * multiplier;

            QString styledText = pair.second.arg(out.mid(textStart, textLength));

            out.replace(capturedStart, capturedLength, styledText);
            insertedTagSymbolsCount += pair.second.length() - 2 - 2 * choppingSignsCount * multiplier;
        }
    }
    return out;
}

/**
 * @brief TextFormatter::applyStyling Applies all styling for the text
 * @param dontShowFormattingSymbols True, if it does not suppose to include formatting symbols into resulting string
 * @return Styled string
 */
QString TextFormatter::applyStyling(bool dontShowFormattingSymbols)
{
    return applyHtmlFontStyling(dontShowFormattingSymbols);
}
