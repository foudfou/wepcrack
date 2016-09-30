#ifndef CRYPTO_SSL_H
#define CRYPTO_SSL_H

#include <stdbool.h>
#include <openssl/evp.h>

#define SSL_OP_ENC 0
#define SSL_OP_DEC 1

#define SHA1_LEN 20 // EVP_MD_size(EVP_sha1())

bool ssl_crypt(int op, const EVP_CIPHER *cypher,
               unsigned char *out, int *out_len,
               const unsigned char *in, const int in_len,
               const unsigned char *key, const int key_len,
               const unsigned char *iv);

bool ssl_digest(const EVP_MD *md,
                unsigned char md_value[EVP_MAX_MD_SIZE], int *md_len,
                const unsigned char *in, const int in_len);

bool ssl_base64(const int op, char *out, int out_max, int *out_len,
                const unsigned char* in, const int in_len);

bool ssl_pbkdf2_sha1(unsigned char *out, const unsigned char *key, int key_len,
                     const unsigned char *data, int data_len, const int iter);

#endif /* CRYPTO_SSL_H */
