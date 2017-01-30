#include "src/core/toxid.h"

#include <QString>

#include <gtest/gtest.h>

class ToxPkTest : public ::testing::Test
{
protected:
    ToxPkTest()
    {
    }

    const uint8_t testPkArray[32] = {
        0xC7, 0x71, 0x9C, 0x68, 0x08, 0xC1, 0x4B, 0x77, 0x34, 0x80, 0x04, 0x95,
        0x6D, 0x1D, 0x98, 0x04, 0x6C, 0xE0, 0x9A, 0x34, 0x37, 0x0E, 0x76, 0x08,
        0x15, 0x0E, 0xAD, 0x74, 0xC3, 0x81, 0x5D, 0x30
    };

    const QString testStr = QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30");
    const QByteArray testPk = QByteArray::fromHex(testStr.toLatin1());

    const QString echoStr = QStringLiteral("76518406F6A9F2217E8DC487CC783C25CC16A15EB36FF32E335A235342C48A39");
    const QByteArray echoPk = QByteArray::fromHex(echoStr.toLatin1());
};

TEST_F(ToxPkTest, toStringTest) {
    ToxPk pk(testPk);
    ASSERT_EQ(testStr, pk.toString());
}

TEST_F(ToxPkTest, equalTest) {
    ToxPk pk1(testPk);
    ToxPk pk2(testPk);
    ToxPk pk3(echoPk);
    ASSERT_EQ(pk1, pk2);
    ASSERT_NE(pk1, pk3);
    bool equal = pk1 == pk2;
    bool nequal = pk1 != pk2;
    ASSERT_NE(nequal, equal);
}

TEST_F(ToxPkTest, clearTest) {
    ToxPk empty;
    ToxPk pk(testPk);
    ASSERT_TRUE(empty.isEmpty());
    ASSERT_FALSE(pk.isEmpty());
}

TEST_F(ToxPkTest, copyTest) {
    ToxPk src(testPk);
    ToxPk copy = src;
    ASSERT_EQ(copy, src);
}

TEST_F(ToxPkTest, testKey) {
    ToxPk pk(testPk);
    ASSERT_EQ(testPk, pk.getKey());
    for (int i = 0; i < 32; i++) {
        ASSERT_EQ(testPkArray[i], pk.getBytes()[i]);
    }
}
