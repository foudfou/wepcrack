#include "test.h"
#include "bit.h"

#define VAL1 (1U << 0)
#define VAL2 (1U << 1)

int main(void)
{
    unsigned int field = 0;
    BIT_SET(field, VAL1);
    ASSERT(BIT_CHK(field, VAL1));

    ASSERT(!BIT_CHK(field, VAL2));

    BIT_TGL(field, VAL1);
    ASSERT(!BIT_CHK(field, VAL1));

    BIT_SET(field, VAL2);
    ASSERT(BIT_CHK(field, VAL2));
    BIT_CLR(field, VAL2);
    ASSERT(!BIT_CHK(field, VAL2));

    return 0;
}
