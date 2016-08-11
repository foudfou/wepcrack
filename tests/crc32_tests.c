/*
 * CRC32 tests from RFC (https://tools.ietf.org/html/rfc3720#appendix-B.4)
 *
 * test vectors from http://dox.ipxe.org/crc32__test_8c.html
 */
#include <string.h>
#include <stdio.h>
#include "crc32.h"

#define TEST_COUNT 4

static unsigned char data[TEST_COUNT][32] = {
    {0},
    {11,'h','e','l','l','o',' ','w','o','r','l','d'},
    {5,'h', 'e', 'l', 'l', 'o'},
    {6,' ', 'w', 'o', 'r', 'l', 'd'}
};

static uint32_t seeds[TEST_COUNT] = {
    0x12345678UL,
    0xffffffffUL,
    0xffffffffUL,
    0xc9ef5979UL
};

static uint32_t crcs[TEST_COUNT] = {
    0x12345678UL,
    0xf2b5ee7aUL,
    0xc9ef5979UL,
    0xf2b5ee7aUL
};

int main(void)
{
    for (int i = 0; i < TEST_COUNT; i++) {
        uint32_t crc = crc32_update(seeds[i], &data[i][1], data[i][0]);
        if (crc != crcs[i]) {
            printf("E");
            return(1);
        }
        printf(".");
    }

    return(0);
}

#undef TEST_COUNT
