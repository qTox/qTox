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

#include "src/persistence/smileypack.h"

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QString>
#include <QStandardPaths>

#include <QGuiApplication>

#include <memory>

static const QString RICH_TEXT_PATTERN = QStringLiteral("<img title=\"%1\" src=\"key:%1\"\\>");

QString getAsRichText(const QString& key);

class TestSmileyPack : public QObject
{
    Q_OBJECT
public:
    TestSmileyPack();

private slots:
    void testSmilifySingleCharEmoji();
    void testSmilifyMultiCharEmoji();
    void testSmilifyAsciiEmoticon();
private:
    std::unique_ptr<QGuiApplication> app;
};

TestSmileyPack::TestSmileyPack()
{
    static char arg1[]{"QToxSmileyPackTestApp"};
    static char arg2[]{"-platform"};
    static char arg3[]{"offscreen"};
    static char* qtTestAppArgv[] = {arg1, arg2, arg3};
    static int qtTestAppArgc = 3;

    app = std::unique_ptr<QGuiApplication>(new QGuiApplication(qtTestAppArgc, qtTestAppArgv));
}

/**
 * @brief Test that single-character emojis (non-ascii) are correctly smilified
 */
void TestSmileyPack::testSmilifySingleCharEmoji()
{
    auto& smileyPack = SmileyPack::getInstance();

    auto result = smileyPack.smileyfied("😊");
    QVERIFY(result == getAsRichText("😊"));

    result = smileyPack.smileyfied("Some😊Letters");
    QVERIFY(result == "Some" + getAsRichText("😊") + "Letters");
}

/**
 * @brief Test that multi-character emojis (non-ascii) are correctly smilified
 *  and not incorrectly matched against single-char counterparts
 */
void TestSmileyPack::testSmilifyMultiCharEmoji()
{
    auto& smileyPack = SmileyPack::getInstance();

    auto result = smileyPack.smileyfied("🇬🇧");
    QVERIFY(result == getAsRichText("🇬🇧"));

    result = smileyPack.smileyfied("Some🇬🇧Letters");
    QVERIFY(result == "Some" + getAsRichText("🇬🇧") + "Letters");

    // This verifies that multi-char emojis are not accidentally
    // considered a multichar ascii smiley
    result = smileyPack.smileyfied("🇫🇷🇬🇧");
    QVERIFY(result == getAsRichText("🇫🇷") + getAsRichText("🇬🇧"));
}


/**
 * @brief Test that single character emojis (non-ascii) are correctly smilified
 *  and not when surrounded by non-punctuation and non-whitespace chars
 */
void TestSmileyPack::testSmilifyAsciiEmoticon()
{
    auto& smileyPack = SmileyPack::getInstance();

    auto result = smileyPack.smileyfied(":-)");
    QVERIFY(result == getAsRichText(":-)"));

    constexpr auto testMsg = "Some:-)Letters";
    result = smileyPack.smileyfied(testMsg);

    // Nothing has changed. Ascii smileys are only considered
    // when they are surrounded by white space
    QVERIFY(result == testMsg);

    result = smileyPack.smileyfied("  :-)  ");
    QVERIFY(result == "  " + getAsRichText(":-)") + "  ");
}


QTEST_GUILESS_MAIN(TestSmileyPack)
#include "smileypack_test.moc"
