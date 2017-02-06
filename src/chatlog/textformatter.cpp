#include "textformatter.h"

#include <QMap>
#include <QPair>
#include <QRegularExpression>
#include <QVector>

enum HtmlFontFormattingStyles {
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

static const QVector<QString> fontStylePatterns = QVector<QString> {
    QStringLiteral("<b>%1</b>"),
    QStringLiteral("<i>%1</i>"),
    QStringLiteral("<u>%1</u>"),
    QStringLiteral("<s>%1</s>"),
    QStringLiteral("<font color=#595959><code>%1</code></font>")
};

// Unfortunately, can't use simple QMap because ordered applying of styles is required
static const QVector<QPair<QString, QString>> textPatternStyle {
    { SINGLE_SLASH_FORMATTING_TEXT_FONT_PATTERN, fontStylePatterns[HtmlFontFormattingStyles::ITALIC] },
    { SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('*'), fontStylePatterns[HtmlFontFormattingStyles::BOLD] },
    { SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('_'), fontStylePatterns[HtmlFontFormattingStyles::UNDERLINE] },
    { SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('~'), fontStylePatterns[HtmlFontFormattingStyles::STRIKE] },
    { SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('`'), fontStylePatterns[HtmlFontFormattingStyles::CODE] },
    { DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('*'), fontStylePatterns[HtmlFontFormattingStyles::BOLD] },
    { DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('/'), fontStylePatterns[HtmlFontFormattingStyles::ITALIC] },
    { DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('_'), fontStylePatterns[HtmlFontFormattingStyles::UNDERLINE] },
    { DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN.arg('~'), fontStylePatterns[HtmlFontFormattingStyles::STRIKE] },
    { MULTILINE_CODE_FORMATTING_TEXT_FONT_PATTERN, fontStylePatterns[HtmlFontFormattingStyles::CODE] }
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
 * @brief TextFormatter::applyHtmlFontStyling Applies styles to the font of text that was passed to the constructor
 * @param dontShowFormattingSymbols True, if it does not suppose to include formatting symbols into resulting string
 * @return Source text with styled font
 */
QString TextFormatter::applyHtmlFontStyling(bool dontShowFormattingSymbols){
    QString out = sourceString;

    int choppingSignsCountMultiplier = dontShowFormattingSymbols ? 0 : 1;

    for (QPair<QString, QString> pair : textPatternStyle) {
        QRegularExpression exp(pair.first);
        QRegularExpressionMatchIterator matchesIterator = exp.globalMatch(out);
        int insertedTagSymbolsCount = 0;

        while (matchesIterator.hasNext()) {
            QRegularExpressionMatch match = matchesIterator.next();

            // Regular expressions may capture one redundant symbol from both sides because of extra check, so we don't need to handle them
            QString firstCheckResult = match.captured(1);
            int firstCheckResultLength = firstCheckResult.isNull() || firstCheckResult.isEmpty() ? 0 : 1;
            int matchStart = match.capturedStart() + firstCheckResultLength + insertedTagSymbolsCount;

            QString lastCheckResult = match.captured(exp.captureCount());
            int lastCheckResultLength = lastCheckResult.isNull() || lastCheckResult.isEmpty() ? 0 : 1;
            int matchLength = match.capturedLength() - firstCheckResultLength - lastCheckResultLength;

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
