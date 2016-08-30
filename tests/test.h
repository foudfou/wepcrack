#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#define ASSERT_BODY { \
        printf(".");  \
    }                 \
    else {            \
        printf("E");  \
        return 1;     \
    }                 \

#define ASSERT(expr) if (expr) ASSERT_BODY

#define ASSERT_CHECK(val, expect, chk) \
    if ((val) chk (expect))            \
        ASSERT_BODY

#define ASSERT_EQUAL(val, expect) ASSERT_CHECK(val, expect, ==)
#define ASSERT_NOT_EQUAL(val, expect) ASSERT_CHECK(val, expect, !=)
#define ASSERT_GT(val, expect) ASSERT_CHECK(val, expect, >)

#endif /* TEST_H */
