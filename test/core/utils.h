#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#define DEFTESTCASE(NAME)                   \
    TCase* tc_##NAME = tcase_create(#NAME); \
    tcase_add_test(tc_##NAME, NAME##Test);  \
    suite_add_tcase(s, tc_##NAME);

#define DEFTESTCASE_SLOW(NAME, TIMEOUT) \
    DEFTESTCASE(NAME)                   \
    tcase_set_timeout(tc_##NAME, TIMEOUT);

#endif // _TEST_UTILS_H_
