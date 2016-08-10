#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

extern uint32_t crc32_tab[256];

void crc32_gentab();

uint32_t crc32_update(const unsigned long crc, const unsigned char *buf,
                      const unsigned int len);

uint32_t crc32(const unsigned char *buf, const unsigned int len);

#endif /* CRC32_H */
