// man 3 EVP_EncryptInit for examples and available cyphers
#include <string.h>
#include "crypto_ssl.h"

bool ssl_crypt(const EVP_CIPHER *cypher,
               unsigned char *out, int *out_len,
               const unsigned char *in, const int in_len,
               const unsigned char *key, const int key_len,
               const unsigned char *iv,
               int encrypt)
{
    bool rv = false;
    int tmp_len;
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);

    EVP_CipherInit_ex(&ctx, cypher, NULL, NULL, NULL, encrypt);
    if (key_len)
        EVP_CIPHER_CTX_set_key_length(&ctx, key_len);
    EVP_CipherInit_ex(&ctx, NULL, NULL, key, iv, encrypt);
    if (!EVP_CipherUpdate(&ctx, out, out_len, in, in_len)) goto cleanup;
    if (!EVP_CipherFinal_ex(&ctx, out + *out_len, &tmp_len)) goto cleanup;
    *out_len += tmp_len;
    rv = true;

  cleanup:
    EVP_CIPHER_CTX_cleanup(&ctx);
    return rv;
}
