#include "test.h"
#include "utils.h"

int main(void)
{
    ASSERT_EQUAL(powull(256, 5), 1099511627776);
    ASSERT_EQUAL(powull(7200, 5), 0); // overflow

    return(0);
}
