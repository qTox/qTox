#include "src/core/toxid.h"

#include <QString>

#include <gtest/gtest.h>

class ToxIdTest : public ::testing::Test
{
protected:
    ToxIdTest()
    {
    }

    const QString corrupted = QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30C8BA3AB9BEBA");
    const QString testToxId = QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30C8BA3AB9BEB9");
    const QString publicKey = QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30");
    const QString echoToxId = QStringLiteral("76518406F6A9F2217E8DC487CC783C25CC16A15EB36FF32E335A235342C48A39218F515C39A6");

};

TEST_F(ToxIdTest, toStringTest) {
    ToxId toxId(testToxId);
    ASSERT_EQ(testToxId, toxId.toString()) << testToxId.toStdString();
}

TEST_F(ToxIdTest, equalTest) {
    ToxId toxId1(testToxId);
    ToxId toxId2(publicKey);
    bool equal = toxId1 == toxId2;
    bool nequal = toxId1 != toxId2;
    ASSERT_EQ(toxId1, toxId2);
    ASSERT_NE(nequal, equal);
}

TEST_F(ToxIdTest, notEqualTest) {
    ToxId toxId1(testToxId);
    ToxId toxId2(echoToxId);
    ASSERT_NE(toxId1, toxId2);
}

TEST_F(ToxIdTest, clearTest) {
    ToxId empty;
    ToxId test(testToxId);
    ASSERT_NE(empty, test);
    test.clear();
    ASSERT_EQ(empty, test);
}

TEST_F(ToxIdTest, copyTest) {
    ToxId src(testToxId);
    ToxId copy = src;
    ASSERT_EQ(copy, src);
}

TEST_F(ToxIdTest, validationCheck) {
    ASSERT_TRUE(ToxId(testToxId).isValid());
    ASSERT_FALSE(ToxId(publicKey).isValid());
    ASSERT_FALSE(ToxId(corrupted).isValid());
    QString deadbeef = "DEADBEEF";
    ASSERT_FALSE(ToxId(deadbeef).isValid());
}
