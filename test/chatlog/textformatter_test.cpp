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
#include <QString>

#include <ctime>
#include <functional>

#define PAIR_FORMAT(input, output) {QStringLiteral(input), QStringLiteral(output)}

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

/**
 * @brief The MarkdownToTags struct maps sequence of markdown symbols to HTML tags according this
 * sequence
 */
struct MarkdownToTags
{
    QString markdownSequence;
    StringPair htmlTags;
};

static const QVector<MarkdownToTags> SINGLE_SIGN_MARKDOWN {
    {QStringLiteral("*"), TAGS[StyleType::BOLD]},
    {QStringLiteral("/"), TAGS[StyleType::ITALIC]},
    {QStringLiteral("_"), TAGS[StyleType::UNDERLINE]},
    {QStringLiteral("~"), TAGS[StyleType::STRIKE]},
    {QStringLiteral("`"), TAGS[StyleType::CODE]},
};

static const QVector<MarkdownToTags> DOUBLE_SIGN_MARKDOWN {
    {QStringLiteral("**"), TAGS[StyleType::BOLD]},
    {QStringLiteral("//"), TAGS[StyleType::ITALIC]},
    {QStringLiteral("__"), TAGS[StyleType::UNDERLINE]},
    {QStringLiteral("~~"), TAGS[StyleType::STRIKE]},
};

static const QVector<MarkdownToTags> MULTI_SIGN_MARKDOWN {
    {QStringLiteral("```"), TAGS[StyleType::CODE]},
};

/**
 * @brief Creates single container from two
 */
template<class Container>
static Container concat(const Container& first, const Container& last)
{
    Container result;
    result.reserve(first.size() + last.size());
    result.append(first);
    result.append(last);
    return result;
}

static const QVector<MarkdownToTags> ALL_MARKDOWN_TYPES = concat(concat(SINGLE_SIGN_MARKDOWN,
                                                                        DOUBLE_SIGN_MARKDOWN),
                                                                 MULTI_SIGN_MARKDOWN);

static const QVector<MarkdownToTags> SINGLE_AND_DOUBLE_MARKDOWN = concat(SINGLE_SIGN_MARKDOWN,
                                                                         DOUBLE_SIGN_MARKDOWN);

// any markdown type must work for this data the same way
static const QVector<StringPair> COMMON_WORK_CASES {
    PAIR_FORMAT("%1a%1", "%2%1a%1%3"),
    PAIR_FORMAT("%1aa%1", "%2%1aa%1%3"),
    PAIR_FORMAT("%1aaa%1", "%2%1aaa%1%3"),
    // Must allow same formatting more than one time
    PAIR_FORMAT("%1aaa%1 %1aaa%1", "%2%1aaa%1%3 %2%1aaa%1%3"),
};

static const QVector<StringPair> SINGLE_SIGN_WORK_CASES {
    PAIR_FORMAT("a %1a%1", "a %2%1a%1%3"),
    PAIR_FORMAT("%1a%1 a", "%2%1a%1%3 a"),
    PAIR_FORMAT("a %1a%1 a", "a %2%1a%1%3 a"),
    // "Lazy" matching
    PAIR_FORMAT("%1aaa%1 aaa%1", "%2%1aaa%1%3 aaa%4"),
};

// only double-sign markdown must work for this data
static const QVector<StringPair> DOUBLE_SIGN_WORK_CASES {
    // Must apply formatting to strings which contain reserved symbols
    PAIR_FORMAT("%1aaa%2%1", "%3%1aaa%2%1%4"),
    PAIR_FORMAT("%1%2aaa%1", "%3%1%2aaa%1%4"),
    PAIR_FORMAT("%1aaa%2aaa%1", "%3%1aaa%2aaa%1%4"),
    PAIR_FORMAT("%1%2%2aaa%1", "%3%1%2%2aaa%1%4"),
    PAIR_FORMAT("%1aaa%2%2%1", "%3%1aaa%2%2%1%4"),
    PAIR_FORMAT("%1aaa%2%2aaa%1", "%3%1aaa%2%2aaa%1%4"),
};

// any type of markdown must fail for this data
static const QVector<QString> COMMON_EXCEPTIONS {
    // No empty formatting string
    QStringLiteral("%1%1"),
    // Formatting text must not start/end with whitespace symbols
    QStringLiteral("%1 %1"), QStringLiteral("%1 a%1"), QStringLiteral("%1a %1"),
    // Formatting string must be enclosed by whitespace symbols, newlines or message start/end
    QStringLiteral("a%1aa%1a"), QStringLiteral("%1aa%1a"), QStringLiteral("a%1aa%1"),
    QStringLiteral("a %1aa%1a"), QStringLiteral("a%1aa%1 a"),
    QStringLiteral("a\n%1aa%1a"), QStringLiteral("a%1aa%1\na"),
};

static const QVector<QString> SINGLE_AND_DOUBLE_SIGN_EXCEPTIONS {
    // No newlines
    QStringLiteral("%1\n%1"), QStringLiteral("%1aa\n%1"), QStringLiteral("%1\naa%1"),
    QStringLiteral("%1aa\naa%1"),
};

// only single-sign markdown must fail for this data
static const QVector<QString> SINGLE_SIGN_EXCEPTIONS {
    // Reserved symbols within formatting string are disallowed
    QStringLiteral("%1aa%1a%1"), QStringLiteral("%1aa%1%1"), QStringLiteral("%1%1aa%1"),
    QStringLiteral("%1%1%1"),
};

static const QVector<StringPair> MIXED_FORMATTING_SPECIAL_CASES {
    // Must allow mixed formatting if there is no tag overlap in result
    PAIR_FORMAT("aaa *aaa /aaa/ aaa*", "aaa <b>aaa <i>aaa</i> aaa</b>"),
    PAIR_FORMAT("aaa *aaa /aaa* aaa/", "aaa *aaa <i>aaa* aaa</i>"),
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

#undef PAIR_FORMAT

using MarkdownFunction = QString (*)(const QString&, bool);
using InputProcessor = std::function<QString(const QString&, const MarkdownToTags&)>;
using OutputProcessor = std::function<QString(const QString&, const MarkdownToTags&, bool)>;

/**
 * @brief Testing cases where markdown must work
 * @param applyMarkdown Function which is used to apply markdown
 * @param markdownToTags Which markdown type to test
 * @param testData Test data - string pairs "Source message - Message after formatting"
 * @param showSymbols True if it is supposed to leave markdown symbols after formatting, false
 * otherwise
 * @param processInput Test data is a template, which must be expanded with concrete markdown
 * symbols, everytime in different way. This function determines how to expand source message
 * depending on user need
 * @param processOutput Same as previous parameter but is applied to markdown output
 */
static void workCasesTest(MarkdownFunction applyMarkdown,
                          const QVector<MarkdownToTags>& markdownToTags,
                          const QVector<StringPair>& testData,
                          bool showSymbols,
                          InputProcessor processInput = nullptr,
                          OutputProcessor processOutput = nullptr)
{
    for (const MarkdownToTags& mtt: markdownToTags) {
        for (const StringPair& data: testData) {
            const QString input = processInput != nullptr ? processInput(data.first, mtt)
                                                          : data.first;
            qDebug() << "Input:" << input;
            QString output = processOutput != nullptr ? processOutput(data.second, mtt, showSymbols)
                                                      : data.second;
            qDebug() << "Expected output:" << output;
            QString result = applyMarkdown(input, showSymbols);
            qDebug() << "Observed output:" << result;
            QVERIFY(output == result);
        }
    }
}

/**
 * @brief Testing cases where markdown must not to work
 * @param applyMarkdown Function which is used to apply markdown
 * @param markdownToTags Which markdown type to test
 * @param exceptions Collection of "source message - markdown result" pairs representing cases
 * where markdown must not to work
 * @param showSymbols True if it is supposed to leave markdown symbols after formatting, false
 * otherwise
 */
static void exceptionsTest(MarkdownFunction applyMarkdown,
                           const QVector<MarkdownToTags>& markdownToTags,
                           const QVector<QString>& exceptions,
                           bool showSymbols)
{
    for (const MarkdownToTags& mtt: markdownToTags) {
        for (const QString& e: exceptions) {
            QString processedException = e.arg(mtt.markdownSequence);
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
    for (const auto& p : pairs) {
        qDebug() << "Input:" << p.first;
        qDebug() << "Expected output:" << p.second;
        QString result = applyMarkdown(p.first, false);
        qDebug() << "Observed output:" << result;
        QVERIFY(p.second == result);
    }
}

class TestTextFormatter : public QObject
{
    Q_OBJECT
private slots:
    void commonWorkCasesShowSymbols();
    void commonWorkCasesHideSymbols();
    void singleSignWorkCasesShowSymbols();
    void singleSignWorkCasesHideSymbols();
    void doubleSignWorkCasesShowSymbols();
    void doubleSignWorkCasesHideSymbols();
    void commonExceptionsShowSymbols();
    void commonExceptionsHideSymbols();
    void singleSignExceptionsShowSymbols();
    void singleSignExceptionsHideSymbols();
    void singleAndDoubleMarkdownExceptionsShowSymbols();
    void singleAndDoubleMarkdownExceptionsHideSymbols();
    void mixedFormattingSpecialCases();
private:
    MarkdownFunction markdownFunction;
};

static QString commonWorkCasesProcessInput(const QString& str, const MarkdownToTags& mtt)
{
    return str.arg(mtt.markdownSequence);
}

static QString commonWorkCasesProcessOutput(const QString& str,
                                            const MarkdownToTags& mtt,
                                            bool showSymbols)
{
    const StringPair& tags = mtt.htmlTags;
    return str.arg(showSymbols ? mtt.markdownSequence : QString{}).arg(tags.first).arg(tags.second);
}

void TestTextFormatter::commonWorkCasesShowSymbols()
{
    workCasesTest(markdownFunction,
                  ALL_MARKDOWN_TYPES,
                  COMMON_WORK_CASES,
                  true,
                  commonWorkCasesProcessInput,
                  commonWorkCasesProcessOutput);
}

void TestTextFormatter::commonWorkCasesHideSymbols()
{
    workCasesTest(markdownFunction,
                  ALL_MARKDOWN_TYPES,
                  COMMON_WORK_CASES,
                  false,
                  commonWorkCasesProcessInput,
                  commonWorkCasesProcessOutput);
}

static QString singleSignWorkCasesProcessInput(const QString& str, const MarkdownToTags& mtt)
{
    return str.arg(mtt.markdownSequence);
}

static QString singleSignWorkCasesProcessOutput(const QString& str,
                                                const MarkdownToTags& mtt,
                                                bool showSymbols)
{
    const StringPair& tags = mtt.htmlTags;
    return str.arg(showSymbols ? mtt.markdownSequence : "")
              .arg(tags.first)
              .arg(tags.second)
              .arg(mtt.markdownSequence);
}

void TestTextFormatter::singleSignWorkCasesShowSymbols()
{
    workCasesTest(markdownFunction,
                  SINGLE_SIGN_MARKDOWN,
                  SINGLE_SIGN_WORK_CASES,
                  true,
                  singleSignWorkCasesProcessInput,
                  singleSignWorkCasesProcessOutput);
}

void TestTextFormatter::singleSignWorkCasesHideSymbols()
{
    workCasesTest(markdownFunction,
                  SINGLE_SIGN_MARKDOWN,
                  SINGLE_SIGN_WORK_CASES,
                  false,
                  singleSignWorkCasesProcessInput,
                  singleSignWorkCasesProcessOutput);
}

static QString doubleSignWorkCasesProcessInput(const QString& str, const MarkdownToTags& mtt)
{
    return str.arg(mtt.markdownSequence).arg(mtt.markdownSequence[0]);
}

static QString doubleSignWorkCasesProcessOutput(const QString& str,
                                                const MarkdownToTags& mtt,
                                                bool showSymbols)
{
    const StringPair& tags = mtt.htmlTags;
    return str.arg(showSymbols ? mtt.markdownSequence : "")
              .arg(mtt.markdownSequence[0])
              .arg(tags.first)
              .arg(tags.second);
}

void TestTextFormatter::doubleSignWorkCasesShowSymbols()
{
    workCasesTest(markdownFunction,
                  DOUBLE_SIGN_MARKDOWN,
                  DOUBLE_SIGN_WORK_CASES,
                  true,
                  doubleSignWorkCasesProcessInput,
                  doubleSignWorkCasesProcessOutput);
}

void TestTextFormatter::doubleSignWorkCasesHideSymbols()
{
    workCasesTest(markdownFunction,
                  DOUBLE_SIGN_MARKDOWN,
                  DOUBLE_SIGN_WORK_CASES,
                  false,
                  doubleSignWorkCasesProcessInput,
                  doubleSignWorkCasesProcessOutput);
}

void TestTextFormatter::commonExceptionsShowSymbols()
{
    exceptionsTest(markdownFunction, ALL_MARKDOWN_TYPES, COMMON_EXCEPTIONS, true);
}

void TestTextFormatter::commonExceptionsHideSymbols()
{
    exceptionsTest(markdownFunction, ALL_MARKDOWN_TYPES, COMMON_EXCEPTIONS, false);
}

void TestTextFormatter::singleSignExceptionsShowSymbols()
{
    exceptionsTest(markdownFunction, SINGLE_SIGN_MARKDOWN, SINGLE_SIGN_EXCEPTIONS, true);
}

void TestTextFormatter::singleSignExceptionsHideSymbols()
{
    exceptionsTest(markdownFunction, SINGLE_SIGN_MARKDOWN, SINGLE_SIGN_EXCEPTIONS, false);
}

void TestTextFormatter::singleAndDoubleMarkdownExceptionsShowSymbols()
{
    exceptionsTest(markdownFunction,
                   SINGLE_AND_DOUBLE_MARKDOWN,
                   SINGLE_AND_DOUBLE_SIGN_EXCEPTIONS,
                   true);
}

void TestTextFormatter::singleAndDoubleMarkdownExceptionsHideSymbols()
{
    exceptionsTest(markdownFunction,
                   SINGLE_AND_DOUBLE_MARKDOWN,
                   SINGLE_AND_DOUBLE_SIGN_EXCEPTIONS,
                   false);
}

void TestTextFormatter::mixedFormattingSpecialCases()
{
    specialCasesTest(markdownFunction, MIXED_FORMATTING_SPECIAL_CASES);
}

QTEST_GUILESS_MAIN(TestTextFormatter)
#include "textformatter_test.moc"

