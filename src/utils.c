#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
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

union int_t intfromstr(const char *str, const int itype, bool *err)
{
    char *endptr = NULL;
    errno = 0;
    union int_t val = { 0 };
    switch (itype) {
    case STRTOINT_ULL:
        val.ull = strtoull(str, &endptr, 10);
        break;
    case STRTOINT_L:
        val.l = strtol(str, &endptr, 10);
        break;
    case STRTOINT_J:
        val.j = strtoimax(str, &endptr, 10);
        break;
    default:
        fprintf(stderr, "Unsupported type %d\n", itype);
        *err = true;
        goto end;
    }
    if (errno) {
        perror("strtoull");
        *err = true;
        goto end;
    }

    if (endptr == str) {
        *err = true;
        goto end;
    }

    *err = false;
  end:
    return val;
}
