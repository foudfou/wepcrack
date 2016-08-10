#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

unsigned long long powull(unsigned long long base, unsigned long long exp);

void tohex(char *dst, const unsigned char *src, size_t len);

void print_hex(const unsigned char *bytes, unsigned len);

bool is_little_endian();

#endif /* UTILS_H */
