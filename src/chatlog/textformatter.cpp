#include "textformatter.h"

const QString TextFormatter::SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN = QStringLiteral("(?:(^|[^\\%1]))(\\%1[^\\s\\%1])([^\\%1\\n]+)([^\\s\\%1]\\%1)(?:($|[^\\%1]))");

//Pattern for escaping slashes from inserted HTML tags
const QString TextFormatter::SINGLE_SLASH_FORMATTING_TEXT_FONT_PATTERN = QStringLiteral("(?:(^|[^/<]))(\\/[^\\s/])([^\\n/]+)([^<\\s/]\\/)(?:($|[^/]))");

const QString TextFormatter::DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN = QStringLiteral("(?:(^|[^\\%1]))([\\%1]{2}[^\\s\\%1])([^\\n]+)([^\\s\\%1][\\%1]{2})(?:($|[^\\%1]))");

const QString TextFormatter::MULTILINE_CODE_FORMATTING_TEXT_FONT_PATTERN = QStringLiteral("(?:(^|[^`]))([`]{3})((\\n|.)+)([`]{3})(?:($|[^`]))");

const QVector<QString> TextFormatter::fontStylePatterns {
    QStringLiteral("<b>%1</b>"),
    QStringLiteral("<i>%1</i>"),
    QStringLiteral("<u>%1</u>"),
    QStringLiteral("<s>%1</s>"),
    QStringLiteral("<font color=#595959><code>%1</code></font>")
};

// Unfortunately, can't use simple QMap because ordered applying of styles is required
const QVector<std::pair<QString, QString>> TextFormatter::textPatternStyle {
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

    for (auto pair = textPatternStyle.begin(); pair != textPatternStyle.end(); ++pair) {
        QRegularExpression exp(pair->first);
        QRegularExpressionMatchIterator matchesIterator = exp.globalMatch(out);
        int insertedTagSymbolsCount = 0;

        while (matchesIterator.hasNext()) {
            QRegularExpressionMatch match = matchesIterator.next();

            // Regular expressions may capture one redundant symbol from both sides because of extra check, so we don't need to handle them
            QString firstCheckResult = match.captured(1);
            int firstCheckResultLength = firstCheckResult.isNull() || firstCheckResult.isEmpty() ? 0 : 1;
            int cleanStylingStringStartPosition = match.capturedStart() + firstCheckResultLength + insertedTagSymbolsCount;

            QString lastCheckResult = match.captured(exp.captureCount());
            int lastCheckResultLength = lastCheckResult.isNull() || lastCheckResult.isEmpty() ? 0 : 1;
            int cleanStylingStringLength = match.capturedLength() - firstCheckResultLength - lastCheckResultLength;

            int choppingSignsCount = patternEscapeSignsCount(out.mid(cleanStylingStringStartPosition, cleanStylingStringLength));
            int operationStartPosition = cleanStylingStringStartPosition + choppingSignsCount;
            int operationLength = cleanStylingStringLength - choppingSignsCount * 2;

            QString styledText = pair->second.arg(out.mid(operationStartPosition, operationLength));

            operationStartPosition = cleanStylingStringStartPosition + choppingSignsCount * choppingSignsCountMultiplier;
            operationLength = cleanStylingStringLength - choppingSignsCount * choppingSignsCountMultiplier * 2;

            out.replace(operationStartPosition, operationLength, styledText);
            insertedTagSymbolsCount += pair->second.length() - 2 - choppingSignsCount * (1 - choppingSignsCountMultiplier) * 2;
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
    QString result = applyHtmlFontStyling(dontShowFormattingSymbols);
    return result;
}
