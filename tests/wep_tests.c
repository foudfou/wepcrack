#include <stdbool.h>
#include "test.h"
#include "wep.h"
#include "wep_data.h"

int main(void)
{
    unsigned char key1[WEP_KEY_LEN] = {'t','u','d','e','s'};
    ASSERT(!wep_check_key_auth(&wep_auth1, key1, WEP_KEY_LEN));
    ASSERT(!wep_check_key_data(&wep_data1, key1, WEP_KEY_LEN));
    ASSERT(wep_check_key_auth(&wep_auth2, key1, WEP_KEY_LEN));
    ASSERT(wep_check_key_data(&wep_data2, key1, WEP_KEY_LEN));

    /* There seem to be A LOT of keys also passing the wep_check_key_data test
     * for a given data:
     * c01c5b9b30 c000fe4cb1 405f49462d 60bc3550e0 610e8f870c a141e60f10
     * 61487d08ec 414f006370 415650013c 818bbd4676 e18fa9a396 81ac2413f8
     * 21af4164a2 41b08c62d6 e1c55a9559 01cbaf37fd e1ce8d57f9 61dbe3918f
     * 41e0e47379 a1e8a0c2c0 81eb7a4c02 61fd09f7f1 2244e27a99 e25949fd57
     * 02b0481ea1 22b30d8e07 a2dae46fbf 03039f294a c308ac811e e31d788b77
     * 43245cbdd5 0356094b26 236572e224 2368695c58 c370804b6c a38d42253c
     * 23a330f069 63ad7b6080 a3af1a9867 e3bf72f49d to start with...
     */
    unsigned char key2[WEP_KEY_LEN] = {0xc0,0x1c,0x5b,0x9b,0x30};
    ASSERT(wep_check_key_data(&wep_data2, key2, WEP_KEY_LEN));
    unsigned char key3[WEP_KEY_LEN] = {0xc0,0x00,0xfe,0x4c,0xb1};
    ASSERT(wep_check_key_data(&wep_data1, key3, WEP_KEY_LEN));

    return(0);
}
