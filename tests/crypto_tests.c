/*
 * ARC4 tests from OpenSSL (test/rc4test.c)
 *
 * Adapted from http://patux.net/crypto_programs/sim/rc4/rc4.c.html
 */
#include <string.h>
#include "test.h"
#include "crypto_ssl.h"

#define CRYPT_TEST_COUNT 6

static unsigned char keys[CRYPT_TEST_COUNT][30] = {
    {8,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef},
    {8,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef},
    {8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {4,0xef,0x01,0x23,0x45},
    {8,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef},
    {4,0xef,0x01,0x23,0x45},
};

static unsigned char data_len[CRYPT_TEST_COUNT] = {8,8,8,20,28,10};
static unsigned char data[CRYPT_TEST_COUNT][30] = {
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
};

static unsigned char output[CRYPT_TEST_COUNT][30] = {
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
};

static char *hello = "Hello world!";
static char hello_len = 12;
static char *sha1_out1 = "\xd3\x48\x6a\xe9\x13\x6e\x78\x56"
    "\xbc\x42\x21\x23\x85\xea\x79\x70\x94\x47\x58\x02";
static char *b64_out1 = "SGVsbG8gd29ybGQh";
static int b64_out1_len = 16;

int main(void)
{
    unsigned char cipher[30];
    unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 };

    bool rv = false;
    for (int i = 0; i < CRYPT_TEST_COUNT; i++) {
        int out_len = 0;
        rv = ssl_crypt(SSL_OP_ENC, EVP_rc4(), cipher, &out_len,
                       data[i], data_len[i], &keys[i][1], keys[i][0], iv);
        ASSERT(rv && memcmp(cipher, output[i], data_len[i]) == 0)
    }

    unsigned char md_value[EVP_MAX_MD_SIZE] = { 0 };
    int md_len = 0;
    int sha1_len = EVP_MD_size(EVP_sha1());
    rv = ssl_digest(EVP_sha1(), md_value, &md_len,
                    (unsigned char*)hello, hello_len);
    ASSERT(rv && memcmp(md_value, sha1_out1, sha1_len) == 0)

    int out_max = 512;
    char out[out_max];
    int out_len = 0;
    rv = ssl_base64(SSL_OP_ENC, out, out_max, &out_len,
                    (unsigned char*)hello, hello_len);
    ASSERT(rv && memcmp(out, b64_out1, out_len) == 0)
    ASSERT_EQUAL(out_len, b64_out1_len)

    rv = ssl_base64(SSL_OP_DEC, out, out_max, &out_len,
                    (unsigned char*)b64_out1, b64_out1_len);
    ASSERT(rv && memcmp(out, hello, out_len) == 0)
    ASSERT_EQUAL(out_len, hello_len)

    rv = ssl_base64(-1, out, out_max, &out_len,
                    (unsigned char*)b64_out1, b64_out1_len);
    ASSERT(!rv)

    return 0;
}

#undef TEST_COUNT
