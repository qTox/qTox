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

#include "src/core/toxid.h"

#include <QtTest/QtTest>
#include <QString>
#include <ctime>

const QString corrupted =
    QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30C8BA3AB9BEBA");
const QString testToxId =
    QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30C8BA3AB9BEB9");
const QString publicKey =
    QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30");
const QString echoToxId =
    QStringLiteral("76518406F6A9F2217E8DC487CC783C25CC16A15EB36FF32E335A235342C48A39218F515C39A6");

class TestToxId : public QObject
{
    Q_OBJECT
private slots:
    void toStringTest();
    void equalTest();
    void notEqualTest();
    void clearTest();
    void copyTest();
    void validationTest();
};

void TestToxId::toStringTest()
{
    ToxId toxId(testToxId);
    QVERIFY(testToxId == toxId.toString());
}

void TestToxId::equalTest()
{
    ToxId toxId1(testToxId);
    ToxId toxId2(publicKey);
    QVERIFY(toxId1 == toxId2);
    QVERIFY(!(toxId1 != toxId2));
}

void TestToxId::notEqualTest()
{
    ToxId toxId1(testToxId);
    ToxId toxId2(echoToxId);
    QVERIFY(toxId1 != toxId2);
}

void TestToxId::clearTest()
{
    ToxId empty;
    ToxId test(testToxId);
    QVERIFY(empty != test);
    test.clear();
    QVERIFY(empty == test);
}

void TestToxId::copyTest()
{
    ToxId src(testToxId);
    ToxId copy = src;
    QVERIFY(copy == src);
}

void TestToxId::validationTest()
{
    QVERIFY(ToxId(testToxId).isValid());
    QVERIFY(!ToxId(publicKey).isValid());
    QVERIFY(!ToxId(corrupted).isValid());
    QString deadbeef = "DEADBEEF";
    QVERIFY(!ToxId(deadbeef).isValid());
}

QTEST_GUILESS_MAIN(TestToxId)
#include "toxid_test.moc"
