// Adapted from http://patux.net/crypto_programs/sim/rc4/rc4.c.html
#include <string.h>
#include <stdio.h>
#include "crypto_ssl.h"

/*
 * ARC4 tests from OpenSSL (test/rc4test.c)
 */

static unsigned char keys[7][30] = {
    {8,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef},
    {8,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef},
    {8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {4,0xef,0x01,0x23,0x45},
    {8,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef},
    {4,0xef,0x01,0x23,0x45},
};

static unsigned char data_len[7] = {8,8,8,20,28,10};
static unsigned char data[7][30] = {
    {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xff},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0xff},
    {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,
     0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,
     0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,
     0x12,0x34,0x56,0x78,0xff},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff},
    {0},
};

static unsigned char output[7][30] = {
    {0x75,0xb7,0x87,0x80,0x99,0xe0,0xc5,0x96,0x00},
    {0x74,0x94,0xc2,0xe7,0x10,0x4b,0x08,0x79,0x00},
    {0xde,0x18,0x89,0x41,0xa3,0x37,0x5d,0x3a,0x00},
    {0xd6,0xa1,0x41,0xa7,0xec,0x3c,0x38,0xdf,
     0xbd,0x61,0x5a,0x11,0x62,0xe1,0xc7,0xba,
     0x36,0xb6,0x78,0x58,0x00},
    {0x66,0xa0,0x94,0x9f,0x8a,0xf7,0xd6,0x89,
     0x1f,0x7f,0x83,0x2b,0xa8,0x33,0xc0,0x0c,
     0x89,0x2e,0xbe,0x30,0x14,0x3c,0xe2,0x87,
     0x40,0x01,0x1e,0xcf,0x00},
    {0xd6,0xa1,0x41,0xa7,0xec,0x3c,0x38,0xdf,0xbd,0x61,0x00},
    {0},
};

int main(void)
{
    unsigned char cipher[30];
    unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 };
    int encrypt = 1;

    for (int i = 0; i < 6; i++) {
        int out_len = 0;
        bool rv = ssl_crypt(EVP_rc4(), cipher, &out_len, data[i], data_len[i],
                            &keys[i][1], keys[i][0], iv, encrypt);

        if (!rv || memcmp(cipher, output[i], data_len[i])) {
            printf("E");
            return(1);
        }

        printf(".");
    }

    return(0);
}
