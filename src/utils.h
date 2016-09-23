#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>

#define STRTOINT_ULL 0
#define STRTOINT_L   1
#define STRTOINT_J   2

#define BOOL2STR(v) v ? "true" : "false"

union int_t {
    long               l;
    unsigned long long ull;
    intmax_t           j;
};

unsigned long long powull(unsigned long long base, unsigned long long exp);

void tohex(char *dst, const unsigned char *src, size_t len);

void print_hex(const unsigned char *bytes, unsigned len);

union int_t intfromstr(const char *str, const int itype, bool *err);

#endif /* UTILS_H */
