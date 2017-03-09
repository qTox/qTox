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

#include <functional>

enum TextStyle
{
    BOLD = 0,
    ITALIC,
    UNDERLINE,
    STRIKE,
    CODE,
    HREF
};

static const QVector<QPair<QString, QString>> MARKDOWN_SYMBOLS_HTML_CODES {
    {"*", "&#42"},
    {"/", "&#47"},
    {"_", "&#95"},
    {"~", "&#126"},
    {"`", "&#96"}
};

static const QString COMMON_PATTERN = QStringLiteral("(?<=^|[^%1<])"
                                                     "[%1]{%2}"
                                                     "(?![%1 \\n])"
                                                     ".+?"
                                                     "(?<![%1< \\n])"
                                                     "[%1]{%2}"
                                                     "(?=$|[^%1])");

static const QString MULTILINE_CODE = QStringLiteral("(?<=^|[^`])"
                                                     "```"
                                                     "(?!`)"
                                                     "(.|\\n)+"
                                                     "(?<!`)"
                                                     "```"
                                                     "(?=$|[^`])");

// Items in vector associated with TextStyle values respectively. Do NOT change this order
static const QVector<QString> htmlPatterns{QStringLiteral("<b>%1</b>"),
                                                QStringLiteral("<i>%1</i>"),
                                                QStringLiteral("<u>%1</u>"),
                                                QStringLiteral("<s>%1</s>"),
                                                QStringLiteral(
                                                    "<font color=#595959><code>%1</code></font>"),
                                                QStringLiteral("<a href=\"%1%2\">%2</a>")};

static const QVector<QPair<QRegularExpression, QString>> textPatternStyle{
    {QRegularExpression(COMMON_PATTERN.arg("*", "1")), htmlPatterns[BOLD]},
    {QRegularExpression(COMMON_PATTERN.arg("/", "1")), htmlPatterns[ITALIC]},
    {QRegularExpression(COMMON_PATTERN.arg("_", "1")), htmlPatterns[UNDERLINE]},
    {QRegularExpression(COMMON_PATTERN.arg("~", "1")), htmlPatterns[STRIKE]},
    {QRegularExpression(COMMON_PATTERN.arg("`", "1")), htmlPatterns[CODE]},
    {QRegularExpression(COMMON_PATTERN.arg("*", "2")), htmlPatterns[BOLD]},
    {QRegularExpression(COMMON_PATTERN.arg("/", "2")), htmlPatterns[ITALIC]},
    {QRegularExpression(COMMON_PATTERN.arg("_", "2")), htmlPatterns[UNDERLINE]},
    {QRegularExpression(COMMON_PATTERN.arg("~", "2")), htmlPatterns[STRIKE]},
    {QRegularExpression(MULTILINE_CODE), htmlPatterns[CODE]}};

static const QVector<QRegularExpression> urlPatterns {
    QRegularExpression("((\\bhttp[s]?://(www\\.)?)|(\\bwww\\.))"
                       "[^. \\n]+\\.[^ \\n]+"),
    QRegularExpression("\\b(ftp|smb)://[^ \\n]+"),
    QRegularExpression("\\bfile://(localhost)?/[^ \\n]+"),
    QRegularExpression("\\btox:[a-zA-Z\\d]{76}"),
    QRegularExpression("\\b(mailto|tox):[^ \\n]+@[^ \\n]+")
};

/**
 * @class TextFormatter
 *
 * @brief This class applies formatting to the text messages, e.g. font styling and URL highlighting
 */

TextFormatter::TextFormatter(const QString& str)
    : message(str)
{
}

/**
 * @brief Counts equal symbols at the beginning of the string
 * @param str Source string
 * @return Amount of equal symbols at the beginning of the string
 */
static int patternSignsCount(const QString& str)
{
    QChar escapeSign = str.at(0);
    int result = 0;
    int length = str.length();
    while (result < length && str[result] == escapeSign) {
        ++result;
    }
    return result;
}

/**
 * @brief Checks HTML tags intersection while applying styles to the message text
 * @param str Checking string
 * @return True, if tag intersection detected
 */
static bool isTagIntersection(const QString& str)
{
    const QRegularExpression TAG_PATTERN("(?<=<)/?[a-zA-Z0-9]+(?=>)");

    int openingTagCount = 0;
    int closingTagCount = 0;

    QRegularExpressionMatchIterator iter = TAG_PATTERN.globalMatch(str);
    while (iter.hasNext()) {
        iter.next().captured()[0] == '/' ? ++closingTagCount : ++openingTagCount;
    }
    return openingTagCount != closingTagCount;
}

/**
 * @brief Applies a function for URL's which can be extracted from passed string
 * @param str String in which we are looking for URL's
 * @param func Function which is applied to URL
 */
static void processUrl(QString& str, std::function<QString(QString&)> func)
{
    for (QRegularExpression exp : urlPatterns) {
        QRegularExpressionMatchIterator iter = exp.globalMatch(str);
        while (iter.hasNext()) {
            QRegularExpressionMatch match = iter.next();
            int startPos = match.capturedStart();
            int length = match.capturedLength();
            QString url = str.mid(startPos, length);
            str.replace(startPos, length, func(url));
        }
    }
}

/**
 * @brief Applies styles to the font of text that was passed to the constructor
 * @param showFormattingSymbols True, if it is supposed to include formatting symbols into resulting
 * string
 */
void TextFormatter::applyHtmlFontStyling(bool showFormattingSymbols)
{
    processUrl(message, [] (QString& str) {
        for (auto p : MARKDOWN_SYMBOLS_HTML_CODES) {
            str.replace(p.first, p.second);
        }
        return str;
    });
    for (QPair<QRegularExpression, QString> pair : textPatternStyle) {
        QRegularExpressionMatchIterator matchesIterator = pair.first.globalMatch(message);
        int insertedTagSymbolsCount = 0;

        while (matchesIterator.hasNext()) {
            QRegularExpressionMatch match = matchesIterator.next();
            if (isTagIntersection(match.captured())) {
                continue;
            }

            int capturedStart = match.capturedStart() + insertedTagSymbolsCount;
            int capturedLength = match.capturedLength();

            QString stylingText = message.mid(capturedStart, capturedLength);
            int choppingSignsCount = showFormattingSymbols ? 0 : patternSignsCount(stylingText);
            int textStart = capturedStart + choppingSignsCount;
            int textLength = capturedLength - 2 * choppingSignsCount;

            QString styledText = pair.second.arg(message.mid(textStart, textLength));

            message.replace(capturedStart, capturedLength, styledText);
            // Subtracting length of "%1"
            insertedTagSymbolsCount += pair.second.length() - 2 - 2 * choppingSignsCount;
        }
    }
    for (auto p : MARKDOWN_SYMBOLS_HTML_CODES) {
        message.replace(p.second, p.first);
    }
}

/**
 * @brief Wraps all found URL's in HTML hyperlink tag
 */
void TextFormatter::wrapUrl()
{
    processUrl(message, [] (QString& str) {
        return htmlPatterns[TextStyle::HREF].arg(str.startsWith("www") ? "http://" : "", str);
    });
}

/**
 * @brief Applies all styling for the text
 * @param showFormattingSymbols True, if it is supposed to include formatting symbols into resulting
 * string
 * @return Styled string
 */
QString TextFormatter::applyStyling(bool showFormattingSymbols)
{
    message.toHtmlEscaped();
    applyHtmlFontStyling(showFormattingSymbols);
    wrapUrl();
    return message;
}
