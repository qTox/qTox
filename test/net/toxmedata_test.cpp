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

#include "src/net/toxmedata.h"

#include <ctime>

#include <QtTest/QtTest>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

const QByteArray testToxId =
    QByteArrayLiteral("\xC7\x71\x9C\x68\x08\xC1\x4B\x77\x34\x80\x04\x95\x6D\x1D\x98\x04"
                      "\x6C\xE0\x9A\x34\x37\x0E\x76\x08\x15\x0E\xAD\x74\xC3\x81\x5D\x30"
                      "\xC8\xBA\x3A\xB9\xBE\xB9");
const QByteArray testPublicKey =
    QByteArrayLiteral("\xC7\x71\x9C\x68\x08\xC1\x4B\x77\x34\x80\x04\x95\x6D\x1D\x98\x04"
                      "\x6C\xE0\x9A\x34\x37\x0E\x76\x08\x15\x0E\xAD\x74\xC3\x81\x5D\x30");

class TestToxmeData : public QObject
{
    Q_OBJECT
private slots:
    void parsePublicKeyTest();
    void encryptedJsonTest();
    void lookupRequestTest();

    void lookup_data();
    void lookup();

    void extractCode_data();
    void extractCode();

    void createAddressRequest();

    void getPassTest_data();
    void getPassTest();

    void deleteAddressRequestTest();
};

/**
 * @brief Test if parse public key works correctly.
 */
void TestToxmeData::parsePublicKeyTest()
{
    ToxmeData data;
    QString publicKeyStr = testPublicKey.toHex();
    QString json = QStringLiteral(R"({"key": "%1"})").arg(publicKeyStr);
    QByteArray result = data.parsePublicKey(json);
    QCOMPARE(result, testPublicKey);

    json = QStringLiteral("Some wrong string");
    result = data.parsePublicKey(json);
    QVERIFY(result.isEmpty());
}

/**
 * @brief Test if generation action json with encrypted payload is correct.
 */
void TestToxmeData::encryptedJsonTest()
{
    ToxmeData data;
    int action = 123;
    QByteArray encrypted{QStringLiteral("sometext").toLatin1()};
    QByteArray nonce{QStringLiteral("nonce").toLatin1()};
    QString text = data.encryptedJson(action, testPublicKey, encrypted, nonce);
    QJsonObject json = QJsonDocument::fromJson(text.toLatin1()).object();

    int actionRes = json["action"].toInt();
    QCOMPARE(actionRes, 123);

    QByteArray pkRes = QByteArray::fromHex(json["public_key"].toString().toLatin1());
    QCOMPARE(pkRes, testPublicKey);

    QByteArray encRes = QByteArray::fromBase64(json["encrypted"].toString().toLatin1());
    QCOMPARE(encRes, encrypted);

    QByteArray nonceRes = QByteArray::fromBase64(json["nonce"].toString().toLatin1());
    QCOMPARE(nonceRes, nonce);
}

/**
 * @brief Test if request for lookup generated correctly.
 */
void TestToxmeData::lookupRequestTest()
{
    ToxmeData data;
    QString json = QStringLiteral(R"({"action":3,"name":"testname"})");
    QString result = data.lookupRequest("testname");
    QCOMPARE(result, json);
}

Q_DECLARE_METATYPE(ToxId)

/**
 * @brief Data function for lookup test.
 */
void TestToxmeData::lookup_data()
{
    qRegisterMetaType<ToxId>("ToxId");
    QTest::addColumn<QString>("input", nullptr);
    QTest::addColumn<ToxId>("result", nullptr);
    QString sToxId = testToxId.toHex();

    QTest::newRow("Valid ToxId") << QStringLiteral(R"({"tox_id": "%1"})").arg(sToxId)
                                 << ToxId(testToxId);

    QTest::newRow("Invalid ToxId") << QStringLiteral(R"({"tox_id": "SomeTextHere"})")
                                   << ToxId{};
    QTest::newRow("Not json") << QStringLiteral("Not json string")
                              << ToxId{};
}

/**
 * @brief Test if ToxId parsed from lookup response correcly.
 */
void TestToxmeData::lookup()
{
    QFETCH(QString, input);
    QFETCH(ToxId, result);
    ToxmeData data;
    ToxId toxId = data.lookup(input);
    QCOMPARE(result, toxId);
}

Q_DECLARE_METATYPE(ToxmeData::ExecCode)

/**
 * @brief Data function for extractCode test.
 */
void TestToxmeData::extractCode_data()
{
    qRegisterMetaType<ToxmeData::ExecCode>("ToxmeData::ExecCode");
    QTest::addColumn<QString>("input", nullptr);
    QTest::addColumn<ToxmeData::ExecCode>("result", nullptr);

    QTest::newRow("Custom code") << QStringLiteral(R"({"c": 123})")
                                 << ToxmeData::ExecCode(123);
    QTest::newRow("Ok code") << QStringLiteral(R"({"c": 0})")
                             << ToxmeData::ExecCode::Ok;

    QTest::newRow("String code") << QStringLiteral(R"({"c": "string here"})")
                                 << ToxmeData::ExecCode::IncorrectResponse;
    QTest::newRow("Invalid code") << QStringLiteral(R"({"c": text})")
                                  << ToxmeData::ExecCode::ServerError;
    QTest::newRow("Not json") << QStringLiteral("Not json string")
                              << ToxmeData::ExecCode::ServerError;
}

/**
 * @brief Test if exec code extracts correctly.
 */
void TestToxmeData::extractCode()
{
    QFETCH(QString, input);
    QFETCH(ToxmeData::ExecCode, result);
    ToxmeData data;
    ToxmeData::ExecCode code = data.extractCode(input);
    QCOMPARE(result, code);
}

/**
 * @brief Test if request for address creation is correct.
 */
void TestToxmeData::createAddressRequest()
{
    ToxmeData data;
    ToxId id{testToxId};
    QString name{"Test address"};
    QString bio{"Bio text"};
    bool keepPrivate = true;
    int timestamp = static_cast<int>(time(nullptr));
    QString text = data.createAddressRequest(id, name, bio, keepPrivate);
    QJsonObject json = QJsonDocument::fromJson(text.toLatin1()).object();

    QByteArray toxIdData = QByteArray::fromHex(json["tox_id"].toString().toLatin1());
    ToxId toxIdRes{toxIdData};
    QCOMPARE(toxIdRes, id);

    QString nameRes = json["name"].toString();
    QCOMPARE(nameRes, name);

    bool privRes = json["privacy"].toInt();
    QCOMPARE(privRes, privRes);

    QString bioRes = json["bio"].toString();
    QCOMPARE(bioRes, bio);

    int timeRes = json["timestamp"].toInt();
    // Test will be failed if `createAddressRequest` will take more
    // than 100 seconds
    QVERIFY(qAbs(timeRes - timestamp) < 100);
}

/**
 * @brief Data function for getPassTest.
 */
void TestToxmeData::getPassTest_data()
{
    qRegisterMetaType<ToxmeData::ExecCode>("ToxmeData::ExecCode");
    QTest::addColumn<QString>("input", nullptr);
    QTest::addColumn<QString>("result", nullptr);
    QTest::addColumn<ToxmeData::ExecCode>("code", nullptr);

    QTest::newRow("Valid password") << QStringLiteral(R"({"password": "123qwe"})")
                                    << QStringLiteral("123qwe")
                                    << ToxmeData::ExecCode::Ok;

    QTest::newRow("Null password") << QStringLiteral(R"({"password": null})")
                                   << QStringLiteral("")
                                   << ToxmeData::ExecCode::Updated;

    QTest::newRow("Valid password with null") << QStringLiteral(R"({"password": "null"})")
                                              << QStringLiteral("null")
                                              << ToxmeData::ExecCode::Ok;

    // ERROR: password value with invalid text, but started with 'null' interpreted as Update
#if 0
    QTest::newRow("Invalid null password") << QStringLiteral(R"({"password": nulla})")
                                           << QStringLiteral("")
                                           << ToxmeData::ExecCode::IncorrectResponse;
#endif

    QTest::newRow("Invalid int password") << QStringLiteral(R"({"password": 123})")
                                          << QStringLiteral("")
                                          << ToxmeData::ExecCode::IncorrectResponse;

    QTest::newRow("Not json") << QStringLiteral("Not json")
                              << QStringLiteral("")
                              << ToxmeData::ExecCode::NoPassword;

}

/**
 * @brief Test if password extraction is correct.
 */
void TestToxmeData::getPassTest()
{
    QFETCH(QString, input);
    QFETCH(QString, result);
    QFETCH(ToxmeData::ExecCode, code);

    ToxmeData data;
    ToxmeData::ExecCode resCode = ToxmeData::ExecCode::Ok;
    QString password = data.getPass(input, resCode);
    QCOMPARE(password, result);
    QCOMPARE(resCode, code);
}

/**
 * @brief Test if request for address deletation generated correct.
 */
void TestToxmeData::deleteAddressRequestTest()
{
    ToxmeData data;
    ToxPk pk{testPublicKey};
    int timestamp = static_cast<int>(time(nullptr));
    QString text = data.deleteAddressRequest(pk);
    QJsonObject json = QJsonDocument::fromJson(text.toLatin1()).object();

    QByteArray pkRes = QByteArray::fromHex(json["public_key"].toString().toLatin1());
    QCOMPARE(pkRes, testPublicKey);

    int timeRes = json["timestamp"].toInt();
    // Test will be failed if `deleteAddressRequest` will take more
    // than 100 seconds
    QVERIFY(qAbs(timeRes - timestamp) < 100);
}

QTEST_GUILESS_MAIN(TestToxmeData)
#include "toxmedata_test.moc"
