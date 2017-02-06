#ifndef TEXTFORMATTER_H
#define TEXTFORMATTER_H

#include <QString>

class TextFormatter
{
private:

    QString sourceString;

    int patternEscapeSignsCount(const QString& str);

    int getCapturedLength(const QRegularExpressionMatch& match, const int exprNumber);

    QString applyHtmlFontStyling(bool dontShowFormattingSymbols);

public:
    explicit TextFormatter(const QString& str);

    QString applyStyling(bool dontShowFormattingSymbols);
};

#endif // TEXTFORMATTER_H
