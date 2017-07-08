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
#include <QList>
#include <QMap>
#include <QString>
#include <QVector>
#include <QVector>

#include <ctime>

using StringToString = QMap<QString, QString>;

static const StringToString signsToTags{{"*", "b"}, {"**", "b"}, {"/", "i"}};

static const StringToString
    commonWorkCases{// Basic
                    {QStringLiteral("%1a%1"), QStringLiteral("<%2>%1a%1</%2>")},
                    {QStringLiteral("%1aa%1"), QStringLiteral("<%2>%1aa%1</%2>")},
                    {QStringLiteral("%1aaa%1"), QStringLiteral("<%2>%1aaa%1</%2>")},

                    // Additional text from both sides - must be enclosed with spaces or newlines!
                    {QStringLiteral("aaa %1a%1"), QStringLiteral("aaa <%2>%1a%1</%2>")},
                    {QStringLiteral("%1a%1 aaa"), QStringLiteral("<%2>%1a%1</%2> aaa")},

                    // Must allow same formatting more than one time, divided by two and more
                    // symbols due to QRegularExpressionIterator
                    {QStringLiteral("%1aaa%1 aaa %1aaa%1"),
                     QStringLiteral("<%2>%1aaa%1</%2> aaa <%2>%1aaa%1</%2>")}};

static const QVector<QString>
    commonExceptions{// No whitespaces near to formatting symbols from both sides
                     QStringLiteral("%1 a%1"), QStringLiteral("%1a %1"),

                     // No newlines
                     QStringLiteral("%1aa\n%1"),};

static const QVector<QString>
    singleSignExceptions{
                     // Only exact combinations of symbols must encapsulate formatting string
                     QStringLiteral("%1%1aaa%1"), QStringLiteral("%1aaa%1%1"),

                     // Do not apply markdown in center of words
                     QStringLiteral("%1aaa%1aa%1"), QStringLiteral("part%1part part%1part"),};

static const StringToString singleSlash{
    // Must work with inserted tags
    {QStringLiteral("/aaa<b>aaa aaa</b>/"), QStringLiteral("<i>aaa<b>aaa aaa</b></i>")}};

static const StringToString doubleSign{
    {QStringLiteral("**aaa * aaa**"), QStringLiteral("<b>aaa * aaa</b>")},
    {QStringLiteral("*****"), QStringLiteral("<b>*</b>")}};

static const StringToString mixedFormatting{
    // Must allow mixed formatting if there is no tag overlap in result
    {QStringLiteral("aaa *aaa /aaa/ aaa*"), QStringLiteral("aaa <b>aaa <i>aaa</i> aaa</b>")},
    {QStringLiteral("aaa *aaa /aaa* aaa/"), QStringLiteral("aaa <b>aaa /aaa</b> aaa/")}};

static const StringToString multilineCode{
    // Must allow newlines
    {QStringLiteral("```int main()\n{\n    return 0;\n}```"),
     QStringLiteral("<font color=#595959><code>int main()\n{\n    return 0;\n}</code></font>")}};

static const StringToString urlCases{
    {QStringLiteral("https://github.com/qTox/qTox/issues/4233"),
     QStringLiteral("<a href=\"https://github.com/qTox/qTox/issues/4233\">"
                    "https://github.com/qTox/qTox/issues/4233</a>")},
    {QStringLiteral("www.youtube.com"),
     QStringLiteral("<a href=\"www.youtube.com\">www.youtube.com</a>")},
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
    {QStringLiteral(
                    "http://site.com/part1/part2 "
                    "http://site.com/part3 "
                    "and one more time "
                    "www.site.com/part1/part2"
                    ),
     QStringLiteral(
                    "<a href=\"http://site.com/part1/part2\">http://site.com/part1/part2</a> "
                    "<a href=\"http://site.com/part3\">http://site.com/part3</a> "
                    "and one more time "
                    "<a href=\"www.site.com/part1/part2\">www.site.com/part1/part2</a>"
                    )},
};

/**
 * @brief Testing cases which are common for all types of formatting except multiline code
 * @param noSymbols True if it's not allowed to show formatting symbols
 * @param map Grouped cases
 * @param signs Combination of formatting symbols
 */
static void commonTest(bool showSymbols, const StringToString map, const QString signs)
{
    for (QString key : map.keys()) {
        QString source = key.arg(signs);
        QString result = map[key].arg(showSymbols ? signs : "", signsToTags[signs]);
        QVERIFY(applyMarkdown(source, showSymbols) == result);
    }
}

/**
 * @brief Testing exception cases
 * @param signs Combination of formatting symbols
 */
static void exceptionsTest(const QVector<QString>& exceptions, const QString signs)
{
    for (QString source : exceptions) {
        QString message = source.arg(signs);
        QVERIFY(applyMarkdown(message, false) == message);
    }
}

/**
 * @brief Testing uncommon cases of markdown
 * @param map Grouped cases
 */
static void specialMarkdownTest(const StringToString map)
{
    for (QString key : map.keys()) {
        QVERIFY(applyMarkdown(key, false) == map[key]);
    }
}

/**
 * @brief Uncommon cases of URLs
 * @param map Grouped cases
 */
static void specialUrlTest(const StringToString map)
{
    for (QString key : map.keys()) {
        QString factResult = highlightURL(key);
        QString expecting = map[key];
        QVERIFY(factResult == expecting);
    }
}

class TestTextFormatter : public QObject
{
    Q_OBJECT
private slots:
    void singleSignNoSymbolsTest();
    void slashNoSymbolsTest();
    void doubleSignNoSymbolsTest();
    void singleSignWithSymbolsTest();
    void slashWithSymbolsTest();
    void doubleSignWithSymbolsTest();
    void singleSignExceptionsTest();
    void slashExceptionsTest();
    void doubleSignExceptionsTest();
    void slashSpecialTest();
    void doubleSignSpecialTest();
    void mixedFormattingTest();
    void multilineCodeTest();
    void urlTest();
};

void TestTextFormatter::singleSignNoSymbolsTest()
{
    commonTest(false, commonWorkCases, "*");
}

void TestTextFormatter::slashNoSymbolsTest()
{
    commonTest(false, commonWorkCases, "/");
}

void TestTextFormatter::doubleSignNoSymbolsTest()
{
    commonTest(false, commonWorkCases, "**");
}

void TestTextFormatter::singleSignWithSymbolsTest()
{
    commonTest(true, commonWorkCases, "*");
}

void TestTextFormatter::slashWithSymbolsTest()
{
    commonTest(true, commonWorkCases, "/");
}

void TestTextFormatter::doubleSignWithSymbolsTest()
{
    commonTest(true, commonWorkCases, "**");
}

void TestTextFormatter::singleSignExceptionsTest()
{
    exceptionsTest(commonExceptions, "*");
    exceptionsTest(singleSignExceptions, "*");
}

void TestTextFormatter::slashExceptionsTest()
{
    exceptionsTest(commonExceptions, "/");
}

void TestTextFormatter::doubleSignExceptionsTest()
{
    exceptionsTest(commonExceptions, "**");
}

void TestTextFormatter::slashSpecialTest()
{
    specialMarkdownTest(singleSlash);
}

void TestTextFormatter::doubleSignSpecialTest()
{
    specialMarkdownTest(doubleSign);
}

void TestTextFormatter::mixedFormattingTest()
{
    specialMarkdownTest(mixedFormatting);
}

void TestTextFormatter::multilineCodeTest()
{
    specialMarkdownTest(multilineCode);
}

void TestTextFormatter::urlTest()
{
    specialUrlTest(urlCases);
}

QTEST_GUILESS_MAIN(TestTextFormatter)
#include "textformatter_test.moc"

