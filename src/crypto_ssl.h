#ifndef OSSCRYPTO_H
#define OSSCRYPTO_H

#include <stdbool.h>
#include <openssl/evp.h>

bool ssl_crypt(const EVP_CIPHER *cypher,
               unsigned char *out, int *out_len,
               const unsigned char *in, const int in_len,
               const unsigned char *key, const int key_len,
               const unsigned char *iv,
               int encrypt);

#endif /* OSSCRYPTO_H */
