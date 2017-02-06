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

static const QString SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN = QStringLiteral("(?:(^|[^\\%1]))(\\%1[^\\s\\%1])([^\\%1\\n]+)([^\\s\\%1]\\%1)(?:($|[^\\%1]))");

//Pattern for escaping slashes from inserted HTML tags
static const QString SINGLE_SLASH_FORMATTING_TEXT_FONT_PATTERN = QStringLiteral("(?:(^|[^/<]))(\\/[^\\s/])([^\\n/]+)([^<\\s/]\\/)(?:($|[^/]))");

static const QString DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN = QStringLiteral("(?:(^|[^\\%1]))([\\%1]{2}[^\\s\\%1])([^\\n]+)([^\\s\\%1][\\%1]{2})(?:($|[^\\%1]))");

static const QString MULTILINE_CODE_FORMATTING_TEXT_FONT_PATTERN = QStringLiteral("(?:(^|[^`]))([`]{3})((\\n|.)+)([`]{3})(?:($|[^`]))");

// Items in vector associated with TextStyle values respectively. Do NOT change this order
static const QVector<QString> fontStylePatterns {
    QStringLiteral("<b>%1</b>"),
    QStringLiteral("<i>%1</i>"),
    QStringLiteral("<u>%1</u>"),
    QStringLiteral("<s>%1</s>"),
    QStringLiteral("<font color=#595959><code>%1</code></font>")
};

// Unfortunately, can't use simple QMap because ordered applying of styles is required
static const QVector<QPair<QRegularExpression, QString>> textPatternStyle {
    { QRegularExpression(SINGLE_SLASH_FORMATTING_TEXT_FONT_PATTERN), fontStylePatterns[ITALIC] },
    { QRegularExpression(SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('*')), fontStylePatterns[BOLD] },
    { QRegularExpression(SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('_')), fontStylePatterns[UNDERLINE] },
    { QRegularExpression(SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('~')), fontStylePatterns[STRIKE] },
    { QRegularExpression(SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('`')), fontStylePatterns[CODE] },
    { QRegularExpression(DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('*')), fontStylePatterns[BOLD] },
    { QRegularExpression(DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('/')), fontStylePatterns[ITALIC] },
    { QRegularExpression(DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('_')), fontStylePatterns[UNDERLINE] },
    { QRegularExpression(DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('~')), fontStylePatterns[STRIKE] },
    { QRegularExpression(MULTILINE_CODE_FORMATTING_TEXT_FONT_PATTERN), fontStylePatterns[CODE] }
};

TextFormatter::TextFormatter(const QString &str)
    : sourceString(str) {}

/**
 * @brief TextFormatter::patternEscapeSignsCount Counts equal symbols at the beginning of the string
 * @param str Source string
 * @return Amount of equal symbols at the beginning of the string
 */
int TextFormatter::patternEscapeSignsCount(const QString &str) {
    QChar escapeSign = str.at(0);
    int result = 0;
    for (const QChar c : str) {
        if (c == escapeSign)
            ++result;
        else
            break;
    }
    return result;
}

/**
 * @brief TextFormatter::getCapturedLength Get length of string captured by subexpression with appropriate checks
 * @param match Global match of QRegularExpression
 * @param exprNumber Number of subexpression
 * @return Length of captured string. If nothing was captured, returns 0
 */
int TextFormatter::getCapturedLength(const QRegularExpressionMatch &match, const int exprNumber) {
    QString captured = match.captured(exprNumber);
    return captured.isNull() || captured.isEmpty() ? 0 : captured.length();
}

/**
 * @brief TextFormatter::applyHtmlFontStyling Applies styles to the font of text that was passed to the constructor
 * @param dontShowFormattingSymbols True, if it does not suppose to include formatting symbols into resulting string
 * @return Source text with styled font
 */
QString TextFormatter::applyHtmlFontStyling(bool dontShowFormattingSymbols){
    QString out = sourceString;
    int choppingSignsCountMultiplier = dontShowFormattingSymbols ? 0 : 1;

    for (QPair<QRegularExpression, QString> pair : textPatternStyle) {
        QRegularExpression exp = pair.first;
        QRegularExpressionMatchIterator matchesIterator = exp.globalMatch(out);
        int insertedTagSymbolsCount = 0;

        while (matchesIterator.hasNext()) {
            QRegularExpressionMatch match = matchesIterator.next();

            // Regular expressions may capture one redundant symbol from both sides because of extra check, so we don't need to handle them
            int firstCheckResultLength = getCapturedLength(match, 1);
            int matchStart = match.capturedStart() + firstCheckResultLength + insertedTagSymbolsCount;
            int matchLength = match.capturedLength() - firstCheckResultLength - getCapturedLength(match, exp.captureCount());

            int choppingSignsCount = patternEscapeSignsCount(out.mid(matchStart, matchLength));
            int textStart = matchStart + choppingSignsCount;
            int textLength = matchLength - choppingSignsCount * 2;

            QString styledText = pair.second.arg(out.mid(textStart, textLength));

            textStart = matchStart + choppingSignsCount * choppingSignsCountMultiplier;
            textLength = matchLength - choppingSignsCount * choppingSignsCountMultiplier * 2;

            out.replace(textStart, textLength, styledText);
            insertedTagSymbolsCount += pair.second.length() - 2 - choppingSignsCount * (1 - choppingSignsCountMultiplier) * 2;
        }
    }
    return out;
}

/**
 * @brief TextFormatter::applyStyling Applies all styling for the text
 * @param dontShowFormattingSymbols True, if it does not suppose to include formatting symbols into resulting string
 * @return Styled string
 */
QString TextFormatter::applyStyling(bool dontShowFormattingSymbols) {
    return applyHtmlFontStyling(dontShowFormattingSymbols);
}
