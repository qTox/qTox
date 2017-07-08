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

#include <QRegularExpression>

// clang-format off
static const QString SINGLE_SIGN_PATTERN = QStringLiteral("(?<=^|\\s|\\n)"
                                                          "[%1]"
                                                          "(?!\\s)"
                                                          "[^%1\\n]+"
                                                          "(?<!\\s)"
                                                          "[%1]"
                                                          "(?=$|\\s|\\n)");

static const QString DOUBLE_SIGN_PATTERN = QStringLiteral("(?<=^|\\s|\\n)"
                                                          "[%1]{2}"
                                                          "(?!\\s)"
                                                          "[^\\n]+"
                                                          "(?<!\\s)"
                                                          "[%1]{2}"
                                                          "(?=$|\\s|\\n)");

static const QString MULTILINE_CODE = QStringLiteral("(?<=^|[^`])"
                                                     "```"
                                                     "(?!`)"
                                                     "(.|\\n)+"
                                                     "(?<!`)"
                                                     "```"
                                                     "(?=$|[^`])");

static const QPair<QRegularExpression, QString> REGEX_TO_WRAPPER[] {
    {QRegularExpression(SINGLE_SIGN_PATTERN.arg('*')), "<b>%1</b>"},
    {QRegularExpression(SINGLE_SIGN_PATTERN.arg('/')), "<i>%1</i>"},
    {QRegularExpression(SINGLE_SIGN_PATTERN.arg('_')), "<u>%1</u>"},
    {QRegularExpression(SINGLE_SIGN_PATTERN.arg('~')), "<s>%1</s>"},
    {QRegularExpression(SINGLE_SIGN_PATTERN.arg('`')),"<font color=#595959><code>%1</code></font>"},
    {QRegularExpression(DOUBLE_SIGN_PATTERN.arg('*')), "<b>%1</b>"},
    {QRegularExpression(DOUBLE_SIGN_PATTERN.arg('/')), "<i>%1</i>"},
    {QRegularExpression(DOUBLE_SIGN_PATTERN.arg('_')), "<u>%1</u>"},
    {QRegularExpression(DOUBLE_SIGN_PATTERN.arg('~')), "<s>%1</s>"},
    {QRegularExpression(MULTILINE_CODE), "<font color=#595959><code>%1</code></font>"}};

static const QString HREF_WRAPPER = QStringLiteral(R"(<a href="%1">%1</a>)");

static const QRegularExpression URL_PATTERNS[] = {
        QRegularExpression("\\b(www\\.|((http[s]?)|ftp)://)\\w+\\S+"),
        QRegularExpression("\\b(file|smb)://([\\S| ]*)"),
        QRegularExpression("\\btox:[a-zA-Z\\d]{76}"),
        QRegularExpression("\\bmailto:\\S+@\\S+\\.\\S+"),
        QRegularExpression("\\btox:\\S+@\\S+")
};

/**
 * @brief Highlights URLs within passed message string
 * @param message Where search for URLs
 * @return Copy of message with highlighted URLs
 */
QString highlightURL(const QString& message)
{
    QString result = message;
    for (QRegularExpression exp : URL_PATTERNS) {
        int startLength = result.length();
        int offset = 0;
        QRegularExpressionMatchIterator iter = exp.globalMatch(result);
        while (iter.hasNext()) {
            QRegularExpressionMatch match = iter.next();
            int startPos = match.capturedStart() + offset;
            int length = match.capturedLength();
            QString wrappedURL = HREF_WRAPPER.arg(match.captured());
            result.replace(startPos, length, wrappedURL);
            offset = result.length() - startLength;
        }
    }
    return result;
}

// clang-format on
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
 * @brief Applies styles to the font of text that was passed to the constructor
 * @param message Formatting string
 * @param showFormattingSymbols True, if it is supposed to include formatting symbols into resulting
 * string
 * @return Copy of message with markdown applied
 */
QString applyMarkdown(const QString& message, bool showFormattingSymbols)
{
    QString result = message;
    for (QPair<QRegularExpression, QString> pair : REGEX_TO_WRAPPER) {
        QRegularExpressionMatchIterator matchesIterator = pair.first.globalMatch(result);
        int insertedTagSymbolsCount = 0;

        while (matchesIterator.hasNext()) {
            QRegularExpressionMatch match = matchesIterator.next();
            if (isTagIntersection(match.captured())) {
                continue;
            }

            int capturedStart = match.capturedStart() + insertedTagSymbolsCount;
            int capturedLength = match.capturedLength();

            QString stylingText = result.mid(capturedStart, capturedLength);
            int choppingSignsCount = showFormattingSymbols ? 0 : patternSignsCount(stylingText);
            int textStart = capturedStart + choppingSignsCount;
            int textLength = capturedLength - 2 * choppingSignsCount;

            QString styledText = pair.second.arg(result.mid(textStart, textLength));

            result.replace(capturedStart, capturedLength, styledText);
            // Subtracting length of "%1"
            insertedTagSymbolsCount += pair.second.length() - 2 - 2 * choppingSignsCount;
        }
    }
    return result;
}
