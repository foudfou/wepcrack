#include <stdbool.h>
#include <stdio.h>
#include "wep.h"
#include "wep_data.h"

#define WEP_IS_EQUAL(keychk, expect) \
    if ((keychk) == (expect)) {      \
        printf(".");                 \
    }                                \
    else {                           \
        printf("E");                 \
        return(1);                   \
    }                                \

int main(void)
{
    unsigned char key[WEP_KEY_LEN+1] = {'t','u','d','e','s',0};
    WEP_IS_EQUAL(wep_check_key_auth(&wep_auth1, key, WEP_KEY_LEN), false);
    WEP_IS_EQUAL(wep_check_key_auth(&wep_auth2, key, WEP_KEY_LEN), true);
    WEP_IS_EQUAL(wep_check_key_data(&wep_data2, key, WEP_KEY_LEN), true);

    return(0);
}
