/*
    Copyright © 2018-2019 by The qTox Project Contributors

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

#include "src/core/toxstring.h"

#include <QtTest/QtTest>
#include <QByteArray>
#include <QString>

class TestToxString : public QObject
{
Q_OBJECT
private slots:
    void QStringTest();
    void QByteArrayTest();
    void uint8_tTest();
    void emptyQStrTest();
    void emptyQByteTest();
    void emptyUINT8Test();
    void nullptrUINT8Test();

private:
    /* Test Strings */
    //"My Test String" - test text
    static const QString testStr;
    static const QByteArray testByte;
    static const uint8_t* testUINT8;
    static const int lengthUINT8;

    //"" - empty test text
    static const QString emptyStr;
    static const QByteArray emptyByte;
    static const uint8_t* emptyUINT8;
    static const int emptyLength;
};


/* Test Strings */
//"My Test String" - test text
const QString TestToxString::testStr = QStringLiteral("My Test String");
const QByteArray TestToxString::testByte = QByteArrayLiteral("My Test String");
const uint8_t* TestToxString::testUINT8 = reinterpret_cast<const uint8_t*>("My Test String");
const int TestToxString::lengthUINT8 = 14;

//"" - empty test text
const QString TestToxString::emptyStr = QStringLiteral("");
const QByteArray TestToxString::emptyByte = QByteArrayLiteral("");
const uint8_t* TestToxString::emptyUINT8 = reinterpret_cast<const uint8_t*>("");
const int TestToxString::emptyLength = 0;

/**
 * @brief Use QString as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::QStringTest()
{
    //Create Test Object with QString constructor
    ToxString test(testStr);

    //Check QString
    QString test_string = test.getQString();
    QVERIFY(testStr == test_string);

    //Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(testByte == test_byte);

    //Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(lengthUINT8 == test_size);
    for (int i = 0; i <= lengthUINT8; i++)
    {
        QVERIFY(testUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use QByteArray as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::QByteArrayTest()
{
    //Create Test Object with QByteArray constructor
    ToxString test(testByte);

    //Check QString
    QString test_string = test.getQString();
    QVERIFY(testStr == test_string);

    //Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(testByte == test_byte);

    //Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(lengthUINT8 == test_size);
    for (int i = 0; i <= lengthUINT8; i++)
    {
        QVERIFY(testUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use uint8t* and size_t as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::uint8_tTest()
{
    //Create Test Object with uint8_t constructor
    ToxString test(testUINT8, lengthUINT8);

    //Check QString
    QString test_string = test.getQString();
    QVERIFY(testStr == test_string);

    //Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(testByte == test_byte);

    //Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(lengthUINT8 == test_size);
    for (int i = 0; i <= lengthUINT8; i++)
    {
        QVERIFY(testUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use empty QString as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::emptyQStrTest()
{
    //Create Test Object with QString constructor
    ToxString test(emptyStr);

    //Check QString
    QString test_string = test.getQString();
    QVERIFY(emptyStr == test_string);

    //Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(emptyByte == test_byte);

    //Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(emptyLength == test_size);
    for (int i = 0; i <= emptyLength; i++)
    {
        QVERIFY(emptyUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use empty QByteArray as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::emptyQByteTest()
{
    //Create Test Object with QByteArray constructor
    ToxString test(emptyByte);

    //Check QString
    QString test_string = test.getQString();
    QVERIFY(emptyStr == test_string);

    //Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(emptyByte == test_byte);

    //Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(emptyLength == test_size);
    for (int i = 0; i <= emptyLength; i++)
    {
        QVERIFY(emptyUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use empty uint8_t as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::emptyUINT8Test()
{
    //Create Test Object with uint8_t constructor
    ToxString test(emptyUINT8, emptyLength);

    //Check QString
    QString test_string = test.getQString();
    QVERIFY(emptyStr == test_string);

    //Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(emptyByte == test_byte);

    //Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(emptyLength == test_size);
    for (int i = 0; i <= emptyLength; i++)
    {
        QVERIFY(emptyUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use nullptr and size_t 5 as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::nullptrUINT8Test()
{
    //Create Test Object with uint8_t constructor
    ToxString test(nullptr, 5);//nullptr and length = 5

    //Check QString
    QString test_string = test.getQString();
    QVERIFY(emptyStr == test_string);

    //Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(emptyByte == test_byte);

    //Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(emptyLength == test_size);
    for (int i = 0; i <= emptyLength; i++)
    {
        QVERIFY(emptyUINT8[i] == test_int[i]);
    }
}

QTEST_GUILESS_MAIN(TestToxString)
#include "toxstring_test.moc"
