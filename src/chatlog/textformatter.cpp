/*
    Copyright © 2017-2019 by The qTox Project Contributors

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
#include <QVector>

// clang-format off

namespace {
// Note: escaping of '\' is only needed because QStringLiteral is broken by linebreak
const QString SINGLE_SIGN_PATTERN = QStringLiteral("(?<=^|\\s)"
                                                          "[%1]"
                                                          "(?!\\s)"
                                                          "([^%1\\n]+?)"
                                                          "(?<!\\s)"
                                                          "[%1]"
                                                          "(?=$|\\s)");

const QString SINGLE_SLASH_PATTERN = QStringLiteral("(?<=^|\\s)"
                                                           "/"
                                                           "(?!\\s)"
                                                           "([^/\\n]+?)"
                                                           "(?<!\\s)"
                                                           "/"
                                                           "(?=$|\\s)");

const QString DOUBLE_SIGN_PATTERN = QStringLiteral("(?<=^|\\s)"
                                                          "[%1]{2}"
                                                          "(?!\\s)"
                                                          "([^\\n]+?)"
                                                          "(?<!\\s)"
                                                          "[%1]{2}"
                                                          "(?=$|\\s)");

const QString MULTILINE_CODE = QStringLiteral("(?<=^|\\s)"
                                                     "```"
                                                     "(?!`)"
                                                     "((.|\\n)+?)"
                                                     "(?<!`)"
                                                     "```"
                                                     "(?=$|\\s)");

#define REGEXP_WRAPPER_PAIR(pattern, wrapper)\
{QRegularExpression(pattern,QRegularExpression::UseUnicodePropertiesOption),QStringLiteral(wrapper)}

const QPair<QRegularExpression, QString> REGEX_TO_WRAPPER[] {
    REGEXP_WRAPPER_PAIR(SINGLE_SLASH_PATTERN, "<i>%1</i>"),
    REGEXP_WRAPPER_PAIR(SINGLE_SIGN_PATTERN.arg('*'), "<b>%1</b>"),
    REGEXP_WRAPPER_PAIR(SINGLE_SIGN_PATTERN.arg('_'), "<u>%1</u>"),
    REGEXP_WRAPPER_PAIR(SINGLE_SIGN_PATTERN.arg('~'), "<s>%1</s>"),
    REGEXP_WRAPPER_PAIR(SINGLE_SIGN_PATTERN.arg('`'), "<font color=#595959><code>%1</code></font>"),
    REGEXP_WRAPPER_PAIR(DOUBLE_SIGN_PATTERN.arg('*'), "<b>%1</b>"),
    REGEXP_WRAPPER_PAIR(DOUBLE_SIGN_PATTERN.arg('/'), "<i>%1</i>"),
    REGEXP_WRAPPER_PAIR(DOUBLE_SIGN_PATTERN.arg('_'), "<u>%1</u>"),
    REGEXP_WRAPPER_PAIR(DOUBLE_SIGN_PATTERN.arg('~'), "<s>%1</s>"),
    REGEXP_WRAPPER_PAIR(MULTILINE_CODE, "<font color=#595959><code>%1</code></font>"),
};

#undef REGEXP_WRAPPER_PAIR

const QString HREF_WRAPPER = QStringLiteral(R"(<a href="%1">%1</a>)");
const QString WWW_WRAPPER = QStringLiteral(R"(<a href="http://%1">%1</a>)");

const QVector<QRegularExpression> WWW_WORD_PATTERN = {
        QRegularExpression(QStringLiteral(R"((?<=^|\s)\S*((www\.)\S+))"))
};

const QVector<QRegularExpression> URI_WORD_PATTERNS = {
    // Note: This does not match only strictly valid URLs, but we broaden search to any string following scheme to
    // allow UTF-8 "IRI"s instead of ASCII-only URLs
    QRegularExpression(QStringLiteral(R"((?<=^|\s)\S*((((http[s]?)|ftp)://)\S+))")),
    QRegularExpression(QStringLiteral(R"((?<=^|\s)\S*((file|smb)://([\S| ]*)))")),
    QRegularExpression(QStringLiteral(R"((?<=^|\s)\S*(tox:[a-zA-Z\d]{76}))")),
    QRegularExpression(QStringLiteral(R"((?<=^|\s)\S*(mailto:\S+@\S+\.\S+))")),
    QRegularExpression(QStringLiteral(R"((?<=^|\s)\S*(magnet:[?]((xt(.\d)?=urn:)|(mt=)|(kt=)|(tr=)|(dn=)|(xl=)|(xs=)|(as=)|(x.))[\S| ]+))")),
    QRegularExpression(QStringLiteral(R"((?<=^|\s)\S*(gemini://\S+))")),
    QRegularExpression(QStringLiteral(R"((?<=^|\s)\S*(ed2k://\|file\|\S+))")),
};


// clang-format on

struct MatchingUri {
    bool valid{false};
    int length{0};
};

// pairs of characters that are ignored when surrounding a URI
const QPair<QString, QString> URI_WRAPPING_CHARS[] = {
        {QString("("), QString(")")},
        {QString("["), QString("]")},
        {QString("&quot;"), QString("&quot;")},
        {QString("'"), QString("'")}
};

// characters which are ignored from the end of URI
const QChar URI_ENDING_CHARS[] = {
        QChar::fromLatin1('?'),
        QChar::fromLatin1('.'),
        QChar::fromLatin1('!'),
        QChar::fromLatin1(':'),
        QChar::fromLatin1(',')
};

/**
 * @brief Strips wrapping characters and ending punctuation from URI
 * @param QRegularExpressionMatch of a word containing a URI
 * @return MatchingUri containing info on the stripped URI
 */
MatchingUri stripSurroundingChars(const QStringRef wrappedUri, const int startOfBareUri)
{
    bool matchFound;
    int curValidationStartPos = 0;
    int curValidationEndPos = wrappedUri.length();
    do {
        matchFound = false;
        for (auto const& surroundChars : URI_WRAPPING_CHARS)
        {
            const int openingCharLength = surroundChars.first.length();
            const int closingCharLength = surroundChars.second.length();
            if (surroundChars.first == wrappedUri.mid(curValidationStartPos, openingCharLength) &&
                surroundChars.second == wrappedUri.mid(curValidationEndPos - closingCharLength, closingCharLength)) {
                curValidationStartPos += openingCharLength;
                curValidationEndPos -= closingCharLength;
                matchFound = true;
                break;
            }
        }
        for (QChar const endChar : URI_ENDING_CHARS) {
            const int charLength = 1;
            if (endChar == wrappedUri.at(curValidationEndPos - charLength)) {
                curValidationEndPos -= charLength;
                matchFound = true;
                break;
            }
        }
    } while (matchFound);
    MatchingUri strippedMatch;
    if (startOfBareUri != curValidationStartPos) {
        strippedMatch.valid = false;
    } else {
        strippedMatch.valid = true;
        strippedMatch.length = curValidationEndPos - startOfBareUri;
    }
    return strippedMatch;
}

/**
 * @brief Wrap substrings matching "patterns" with "wrapper" in "message"
 * @param message Where search for patterns
 * @param patterns Array of regex patterns to find strings to wrap
 * @param wrapper Surrounds the matched strings
 * @note done separately from URI since the link must have a scheme added to be valid
 * @return Copy of message with highlighted URLs
 */
QString highlight(const QString& message, const QVector<QRegularExpression>& patterns, const QString& wrapper)
{
    QString result = message;
    for (const QRegularExpression& exp : patterns) {
        const int startLength = result.length();
        int offset = 0;
        QRegularExpressionMatchIterator iter = exp.globalMatch(result);
        while (iter.hasNext()) {
            const QRegularExpressionMatch match = iter.next();
            const int uriWithWrapMatch{0};
            const int uriWithoutWrapMatch{1};
            MatchingUri matchUri = stripSurroundingChars(match.capturedRef(uriWithWrapMatch),
                   match.capturedStart(uriWithoutWrapMatch) - match.capturedStart(uriWithWrapMatch));
            if (!matchUri.valid) {
                continue;
            }
            const QString wrappedURL = wrapper.arg(match.captured(uriWithoutWrapMatch).left(matchUri.length));
            result.replace(match.capturedStart(uriWithoutWrapMatch) + offset, matchUri.length, wrappedURL);
            offset = result.length() - startLength;
        }
    }
    return result;
}

/**
 * @brief Checks HTML tags intersection while applying styles to the message text
 * @param str Checking string
 * @return True, if tag intersection detected
 */
bool isTagIntersection(const QString& str)
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
} // namespace

/**
 * @brief Highlights URLs within passed message string
 * @param message Where search for URLs
 * @return Copy of message with highlighted URLs
 */
QString TextFormatter::highlightURI(const QString& message)
{
    QString result = highlight(message, URI_WORD_PATTERNS, HREF_WRAPPER);
    result = highlight(result, WWW_WORD_PATTERN, WWW_WRAPPER);
    return result;
}

/**
 * @brief Applies markdown to passed message string
 * @param message Formatting string
 * @param showFormattingSymbols True, if it is supposed to include formatting symbols into resulting
 * string
 * @return Copy of message with markdown applied
 */
QString TextFormatter::applyMarkdown(const QString& message, bool showFormattingSymbols)
{
    QString result = message;
    for (const QPair<QRegularExpression, QString>& pair : REGEX_TO_WRAPPER) {
        QRegularExpressionMatchIterator iter = pair.first.globalMatch(result);
        int offset = 0;
        while (iter.hasNext()) {
            const QRegularExpressionMatch match = iter.next();
            QString captured = match.captured(!showFormattingSymbols);
            if (isTagIntersection(captured)) {
                continue;
            }

            const int length = match.capturedLength();
            const QString wrappedText = pair.second.arg(captured);
            const int startPos = match.capturedStart() + offset;
            result.replace(startPos, length, wrappedText);
            offset += wrappedText.length() - length;
        }
    }
    return result;
}
