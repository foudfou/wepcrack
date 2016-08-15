#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

/* Compute the power base^exp over an unsigned long long.
 * Returns 0 in case of overflow.
 */
unsigned long long powull(unsigned long long base, unsigned long long exp)
{
    unsigned long long result = 1;
    while (exp) {
        if (ULLONG_MAX / base < result) // overflow
            return 0;
        result *= base;
        exp--;
    }
    return result;
}

void tohex(char *dst, const unsigned char *src, size_t len)
{
    for (size_t i = 0; i < len; i++)
        snprintf(&(dst[2*i]), 3, "%02x", src[i]);
    dst[2*len+1] = 0;
}

void print_hex(const unsigned char *bytes, unsigned len)
{
    char dst[2*len+1];
    tohex(dst, bytes, len);
    printf("%s\n", dst);
}

bool is_little_endian()
{
    uint32_t endianness = 0x00ff00ff;
    return (((unsigned char *)&endianness)[0] == 0xff);
}
