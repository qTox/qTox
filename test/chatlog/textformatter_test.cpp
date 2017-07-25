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

#include "src/chatlog/textformatter.h"

#include <QtTest/QtTest>
#include <QMap>
#include <QString>

#include <ctime>
#include <functional>

#define PAIR_FORMAT(input, output) {QStringLiteral(input), QStringLiteral(output)}

using StringToString = QMap<QString, QString>;

using StringPair = QPair<QString, QString>;

static const StringPair TAGS[] {
    PAIR_FORMAT("<b>", "</b>"),
    PAIR_FORMAT("<i>", "</i>"),
    PAIR_FORMAT("<u>", "</u>"),
    PAIR_FORMAT("<s>", "</s>"),
    PAIR_FORMAT("<font color=#595959><code>", "</code></font>"),
};

enum StyleType {
    BOLD,
    ITALIC,
    UNDERLINE,
    STRIKE,
    CODE,
};

static const QPair<QString, const StringPair&> SEQUENCE_TO_TAG[] {
    {QStringLiteral("*"), TAGS[StyleType::BOLD]},
    {QStringLiteral("/"), TAGS[StyleType::ITALIC]},
    {QStringLiteral("_"), TAGS[StyleType::UNDERLINE]},
    {QStringLiteral("~"), TAGS[StyleType::STRIKE]},
    {QStringLiteral("`"), TAGS[StyleType::CODE]},
    {QStringLiteral("**"), TAGS[StyleType::BOLD]},
    {QStringLiteral("//"), TAGS[StyleType::ITALIC]},
    {QStringLiteral("__"), TAGS[StyleType::UNDERLINE]},
    {QStringLiteral("~~"), TAGS[StyleType::STRIKE]},
    {QStringLiteral("```"), TAGS[StyleType::CODE]},
};

static const QVector<StringPair> COMMON_WORK_CASES {
    PAIR_FORMAT("%1a%1", "%2%1a%1%3"),
    PAIR_FORMAT("%1aa%1", "%2%1aa%1%3"),
    PAIR_FORMAT("%1aaa%1", "%2%1aaa%1%3"),
    // Must allow same formatting more than one time
    PAIR_FORMAT("%1aaa%1 %1aaa%1", "%2%1aaa%1%3 %2%1aaa%1%3"),
    // "Lazy" matching
    PAIR_FORMAT("%1aaa%1 aaa%1", "%2%1aaa%1%3 aaa%1"),
};

static const QVector<StringPair> DOUBLE_SIGN_WORK_CASES {
    // Must apply formatting to strings which contain reserved symbols
    PAIR_FORMAT("%1%2%1", "%3%1%2%1%4"),
    PAIR_FORMAT("%1%2%2%1", "%3%1%2%2%1%4"),
    PAIR_FORMAT("%1aaa%2%1", "%3%1aaa%2%1%4"),
    PAIR_FORMAT("%1%2aaa%1", "%3%1%2aaa%1%4"),
    PAIR_FORMAT("%1aaa%2aaa%1", "%3%1aaa%2aaa%1%4"),
    PAIR_FORMAT("%1%2%2aaa%1", "%3%1%2%2aaa%1%4"),
    PAIR_FORMAT("%1aaa%2%2%1", "%3%1aaa%2%2%1%4"),
    PAIR_FORMAT("%1aaa%2%2aaa%1", "%3%1aaa%2%2aaa%1%4"),
};

static const QVector<QString> COMMON_EXCEPTIONS {
    // No empty formatting string
    QStringLiteral("%1%1"),
    // Formatting text must not start/end with whitespace symbols
    QStringLiteral("%1 %1"), QStringLiteral("%1 a%1"), QStringLiteral("%1a %1"),
    // No newlines
    QStringLiteral("%1\n%1"), QStringLiteral("%1aa\n%1"), QStringLiteral("%1\naa%1"),
    QStringLiteral("%1aa\naa%1"),
    // Formatting string must be enclosed by whitespace symbols, newlines or message start/end
    QStringLiteral("a%1aa%1a"), QStringLiteral("%1aa%1a"), QStringLiteral("a%1aa%1"),
    QStringLiteral("a %1aa%1a"), QStringLiteral("a%1aa%1 a"),
    QStringLiteral("a\n%1aa%1a"), QStringLiteral("a%1aa%1\na"),
};

static const QVector<QString> SINGLE_SIGN_EXCEPTIONS {
    // Reserved symbols within formatting string are disallowed
    QStringLiteral("%1aa%1a%1"), QStringLiteral("%1aa%1%1"), QStringLiteral("%1%1aa%1"),
    QStringLiteral("%1%1%1"),
};

static const QVector<StringPair> SINGLE_SLASH_SPECIAL_CASES {
    // Must work with inserted tags
    PAIR_FORMAT("/aaa<b>aaa aaa</b>/", "<i>aaa<b>aaa aaa</b></i>"),
};

static const QVector<StringPair> MULTILINE_CODE_SPECIAL_CASES {
    // Must allow newlines
    PAIR_FORMAT("```int main()\n{\n    return 0;\n}```",
                "<font color=#595959><code>int main()\n{\n    return 0;\n}</code></font>"),
};

static const QVector<StringPair> MIXED_FORMATTING_SPECIAL_CASES {
    // Must allow mixed formatting if there is no tag overlap in result
    PAIR_FORMAT("aaa *aaa /aaa/ aaa*", "aaa <b>aaa <i>aaa</i> aaa</b>"),
    PAIR_FORMAT("aaa *aaa /aaa* aaa/", "aaa <b>aaa /aaa</b> aaa/"),
};

static const StringToString urlCases{
    {QStringLiteral("https://github.com/qTox/qTox/issues/4233"),
     QStringLiteral("<a href=\"https://github.com/qTox/qTox/issues/4233\">"
                    "https://github.com/qTox/qTox/issues/4233</a>")},
    {QStringLiteral("No conflicts with /italic https://github.com/qTox/qTox/issues/4233 font/"),
     QStringLiteral("No conflicts with <i>italic "
                    "<a href=\"https://github.com/qTox/qTox/issues/4233\">"
                    "https://github.com/qTox/qTox/issues/4233</a> font</i>")},
    {QStringLiteral("www.youtube.com"),
     QStringLiteral("<a href=\"http://www.youtube.com\">www.youtube.com</a>")},
    {QStringLiteral("https://url.com/some*url/some*more*url/"),
     QStringLiteral("<a href=\"https://url.com/some*url/some*more*url/\">"
                    "https://url.com/some*url/some*more*url/</a>")},
    {QStringLiteral("https://url.com/some_url/some_more_url/"),
     QStringLiteral("<a href=\"https://url.com/some_url/some_more_url/\">"
                    "https://url.com/some_url/some_more_url/</a>")},
    {QStringLiteral("https://url.com/some~url/some~more~url/"),
     QStringLiteral("<a href=\"https://url.com/some~url/some~more~url/\">"
                    "https://url.com/some~url/some~more~url/</a>")},
    {QStringLiteral("https://url.com/some`url/some`more`url/"),
     QStringLiteral("<a href=\"https://url.com/some`url/some`more`url/\">"
                    "https://url.com/some`url/some`more`url/</a>")},
    // Test case from issue #4275
    {QStringLiteral("http://www.metacritic.com/game/pc/mass-effect-andromeda\n"
                    "http://www.metacritic.com/game/playstation-4/mass-effect-andromeda\n"
                    "http://www.metacritic.com/game/xbox-one/mass-effect-andromeda"),
     QStringLiteral("<a href=\"http://www.metacritic.com/game/pc/mass-effect-andromeda\">"
                    "http://www.metacritic.com/game/pc/mass-effect-andromeda</a>\n"
                    "<a href=\"http://www.metacritic.com/game/playstation-4/mass-effect-andromeda\""
                    ">http://www.metacritic.com/game/playstation-4/mass-effect-andromeda</a>\n"
                    "<a href=\"http://www.metacritic.com/game/xbox-one/mass-effect-andromeda\">"
                    "http://www.metacritic.com/game/xbox-one/mass-effect-andromeda</a>")},
    {QStringLiteral("http://site.com/part1/part2 "
                    "http://site.com/part3 "
                    "and one more time "
                    "www.site.com/part1/part2"),
     QStringLiteral("<a href=\"http://site.com/part1/part2\">http://site.com/part1/part2</a> "
                    "<a href=\"http://site.com/part3\">http://site.com/part3</a> "
                    "and one more time "
                    "<a href=\"http://www.site.com/part1/part2\">www.site.com/part1/part2</a>")},
};

using MarkdownFunction = QString (*)(const QString&, bool);
using InputProcessor = std::function<QString(const QString&,
                                             const QPair<QString, const StringPair&>&)>;
using OutputProcessor = std::function<QString(const QString&,
                                              const QPair<QString, const StringPair&>&,
                                              bool)>;

/**
 * @brief Testing cases where markdown must work
 * @param applyMarkdown Function which is used to apply markdown
 * @param pairs Collection of "source message - markdown result" pairs representing cases where
 * markdown must not to work
 * @param showSymbols True if it is supposed to leave markdown symbols after formatting, false
 * otherwise
 * @param processInput Test data is a template, which must be expanded with concrete markdown
 * symbols, everytime in different way. This function determines how to expand source message
 * depending on user need
 * @param processOutput Same as previous parameter but is applied to markdown output
 */
static void workCasesTest(MarkdownFunction applyMarkdown,
                          const QVector<StringPair>& pairs,
                          bool showSymbols,
                          InputProcessor processInput = nullptr,
                          OutputProcessor processOutput = nullptr)
{
    for (auto st : SEQUENCE_TO_TAG) {
        for (auto p : pairs) {
            QString input = processInput != nullptr ? processInput(p.first, st)
                                                    : p.first;
            qDebug() << "Input: " << input;
            QString output = processOutput != nullptr ? processOutput(p.second, st, showSymbols)
                                                      : p.second;
            qDebug() << "Output: " << output;
            QVERIFY(output == applyMarkdown(input, showSymbols));
        }
    }
}

/**
 * @brief Testing cases where markdown must not to work
 * @param applyMarkdown Function which is used to apply markdown
 * @param exceptions Collection of "source message - markdown result" pairs representing cases
 * where markdown must not to work
 * @param showSymbols True if it is supposed to leave markdown symbols after formatting, false
 * otherwise
 */
static void exceptionsTest(MarkdownFunction applyMarkdown,
                           const QVector<QString>& exceptions,
                           bool showSymbols)
{
    for (auto st : SEQUENCE_TO_TAG) {
        for (auto e : exceptions) {
            QString processedException = e.arg(st.first);
            qDebug() << "Exception: " << processedException;
            QVERIFY(processedException == applyMarkdown(processedException, showSymbols));
        }
    }
}

/**
 * @brief Testing some uncommon work cases
 * @param applyMarkdown Function which is used to apply markdown
 * @param pairs Collection of "source message - markdown result" pairs representing cases where
 * markdown must not to work
 */
static void specialCasesTest(MarkdownFunction applyMarkdown,
                             const QVector<StringPair>& pairs)
{
    for (auto p : pairs) {
        qDebug() << "Input: " << p.first;
        qDebug() << "Output: " << p.second;
        QVERIFY(p.second == applyMarkdown(p.first, false));
    }
}

class TestTextFormatter : public QObject
{
    Q_OBJECT
private slots:
    void commonWorkCases();
    void doubleSignWorkCases();
    void commonExceptions();
    void singleSignExceptions();
    void singleSlashSpecialCases();
    void multilineCodeSpecialCases();
    void mixedFormattingSpecialCases();
private:
    MarkdownFunction markdownFunction;
};

void TestTextFormatter::commonWorkCases()
{
    auto processInput = [] (const QString& str, const QPair<QString, const StringPair&>& pair)
    {
        return str.arg(pair.first);
    };
    auto processOutput = [] (const QString& str,
                             const QPair<QString, const StringPair&>& pair,
                             bool showSymbols)
    {
        StringPair tags = pair.second;
        return str.arg(showSymbols ? pair.first : "").arg(tags.first).arg(tags.second);
    };
    workCasesTest(markdownFunction, COMMON_WORK_CASES, true, processInput, processOutput);
    workCasesTest(markdownFunction, COMMON_WORK_CASES, false, processInput, processOutput);
}

void TestTextFormatter::doubleSignWorkCases()
{
    auto processInput = [] (const QString& str, const QPair<QString, const StringPair&>& pair)
    {
        QString symbols = pair.first;
        return str.arg(symbols).arg(symbols[0]);
    };
    auto processOutput = [] (const QString& str,
                             const QPair<QString, const StringPair&>& pair,
                             bool showSymbols)
    {
        QString symbols = pair.first;
        StringPair tags = pair.second;
        return str.arg(showSymbols ? symbols : "").arg(symbols[0]).arg(tags.first).arg(tags.second);
    };
    workCasesTest(markdownFunction, DOUBLE_SIGN_WORK_CASES, true, processInput, processOutput);
    workCasesTest(markdownFunction, DOUBLE_SIGN_WORK_CASES, false, processInput, processOutput);
}

void TestTextFormatter::commonExceptions()
{
    exceptionsTest(markdownFunction, COMMON_EXCEPTIONS, true);
    exceptionsTest(markdownFunction, COMMON_EXCEPTIONS, false);
}

void TestTextFormatter::singleSignExceptions()
{
    exceptionsTest(markdownFunction, SINGLE_SIGN_EXCEPTIONS, true);
    exceptionsTest(markdownFunction, SINGLE_SIGN_EXCEPTIONS, false);
}

void TestTextFormatter::singleSlashSpecialCases()
{
    specialCasesTest(markdownFunction, SINGLE_SLASH_SPECIAL_CASES);
}

void TestTextFormatter::multilineCodeSpecialCases()
{
    specialCasesTest(markdownFunction, MULTILINE_CODE_SPECIAL_CASES);
}

void TestTextFormatter::mixedFormattingSpecialCases()
{
    specialCasesTest(markdownFunction, MIXED_FORMATTING_SPECIAL_CASES);
}

QTEST_GUILESS_MAIN(TestTextFormatter)
#include "textformatter_test.moc"

