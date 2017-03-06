#include "src/core/toxid.h"

#include "test/common.h"

#include <QString>
#include <check.h>

const QString corrupted = QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30C8BA3AB9BEBA");
const QString testToxId = QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30C8BA3AB9BEB9");
const QString publicKey = QStringLiteral("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30");
const QString echoToxId = QStringLiteral("76518406F6A9F2217E8DC487CC783C25CC16A15EB36FF32E335A235342C48A39218F515C39A6");

START_TEST(toStringTest)
{
    ToxId toxId(testToxId);
    ck_assert(testToxId == toxId.toString());
}
END_TEST

START_TEST(equalTest)
{
    ToxId toxId1(testToxId);
    ToxId toxId2(publicKey);
    ck_assert(toxId1 == toxId2);
    ck_assert(!(toxId1 != toxId2));
}
END_TEST

START_TEST(notEqualTest)
{
    ToxId toxId1(testToxId);
    ToxId toxId2(echoToxId);
    ck_assert(toxId1 != toxId2);
}
END_TEST

START_TEST(clearTest)
{
    ToxId empty;
    ToxId test(testToxId);
    ck_assert(empty != test);
    test.clear();
    ck_assert(empty == test);
}
END_TEST

START_TEST(copyTest)
{
    ToxId src(testToxId);
    ToxId copy = src;
    ck_assert(copy == src);
}
END_TEST

START_TEST(validationTest)
{
    ck_assert(ToxId(testToxId).isValid());
    ck_assert(!ToxId(publicKey).isValid());
    ck_assert(!ToxId(corrupted).isValid());
    QString deadbeef = "DEADBEEF";
    ck_assert(!ToxId(deadbeef).isValid());
}
END_TEST

static Suite *toxIdSuite(void)
{
    Suite *s = suite_create("ToxId");

    DEFTESTCASE(toString);
    DEFTESTCASE(equal);
    DEFTESTCASE(notEqual);
    DEFTESTCASE(clear);
    DEFTESTCASE(copy);
    DEFTESTCASE(validation);

    return s;
}

int main(int argc, char *argv[])
{
    srand((unsigned int) time(NULL));

    Suite *toxId = toxIdSuite();
    SRunner *runner = srunner_create(toxId);
    srunner_run_all(runner, CK_NORMAL);

    int res = srunner_ntests_failed(runner);
    srunner_free(runner);

    return res;
}
