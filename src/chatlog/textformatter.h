#ifndef TEXTFORMATTER_H
#define TEXTFORMATTER_H

#include <QMap>
#include <QRegularExpression>
#include <QString>
#include <QVector>

class TextFormatter
{
private:
    enum HtmlFontFormattingStyles {
        BOLD = 0,
        ITALIC,
        UNDERLINE,
        STRIKE,
        CODE
    };

    static const QString SINGLE_SIGN_FORMATTING_TEXT_FONT_PATTERN;

    static const QString SINGLE_SLASH_FORMATTING_TEXT_FONT_PATTERN;

    static const QString DOUBLE_SIGN_FORMATTING_TEXT_FONT_PATTERN;

    static const QString MULTILINE_CODE_FORMATTING_TEXT_FONT_PATTERN;

    static const QVector<QString> fontStylePatterns;

    static const QVector<std::pair<QString, QString>> textPatternStyle;

    QString sourceString;

    int patternEscapeSignsCount(const QString& str);

    QString applyHtmlFontStyling(bool dontShowFormattingSymbols);

public:
    explicit TextFormatter(const QString& str);

    QString applyStyling(bool dontShowFormattingSymbols);
};

#endif // TEXTFORMATTER_H
