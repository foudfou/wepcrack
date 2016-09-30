// man 3 EVP_EncryptInit for examples and available cyphers
#include <string.h>
#include <openssl/bio.h>
#include <openssl/hmac.h>
#include "crypto_ssl.h"

bool ssl_crypt(int op, const EVP_CIPHER *cypher,
               unsigned char *out, int *out_len,
               const unsigned char *in, const int in_len,
               const unsigned char *key, const int key_len,
               const unsigned char *iv)
{
    bool rv = false;
    int tmp_len;
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);

    EVP_CipherInit_ex(&ctx, cypher, NULL, NULL, NULL, op);
    if (key_len)
        EVP_CIPHER_CTX_set_key_length(&ctx, key_len);
    EVP_CipherInit_ex(&ctx, NULL, NULL, key, iv, op);
    if (!EVP_CipherUpdate(&ctx, out, out_len, in, in_len)) goto cleanup;
    if (!EVP_CipherFinal_ex(&ctx, out + *out_len, &tmp_len)) goto cleanup;
    *out_len += tmp_len;
    rv = true;

  cleanup:
    EVP_CIPHER_CTX_cleanup(&ctx);
    return rv;
}


bool ssl_digest(const EVP_MD *md,
                unsigned char md_value[EVP_MAX_MD_SIZE], int *md_len,
                const unsigned char *in, const int in_len)
{
    bool rv = false;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, in, in_len);
    if (!EVP_DigestFinal_ex(mdctx, md_value, (unsigned int*)md_len))
        goto cleanup;
    rv = true;

  cleanup:
    EVP_MD_CTX_destroy(mdctx);
    return rv;
}

bool ssl_base64(const int op, char *out, int out_max, int *out_len,
                const unsigned char* in, const int in_len)
{
    bool rv = false;
    BIO *b64 = BIO_new(BIO_f_base64());          // filter
    BIO *bio = BIO_new(BIO_s_mem());             // source/sink
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);  // single-line data
    BIO_push(b64, bio);
    if (op == SSL_OP_ENC) {
        if (BIO_write(b64, in, in_len) <= 0)
            goto cleanup;
        BIO_flush(b64);
        *out_len = BIO_read(bio, out, out_max);
        if (*out_len <= 0)
            goto cleanup;
    }
    else if (op == SSL_OP_DEC) {
        if (BIO_write(bio, in, in_len) <= 0)
            goto cleanup;
        BIO_flush(bio);
        *out_len = BIO_read(b64, out, out_max);
        if (*out_len <= 0)
            goto cleanup;
    }
    else {
        fprintf(stderr, "Unsupported operation");
        goto cleanup;
    }
    rv = true;

  cleanup:
    BIO_free_all(b64);
    return rv;
}

/* SCRAM customized PBKDF2-HMAC-SHA1
   https://tools.ietf.org/html/rfc5802#section-2.2 */
bool ssl_pbkdf2_sha1(unsigned char *out, const unsigned char *pass, int pass_len,
                     const unsigned char *salt, int salt_len, const int iter)
{
    const unsigned char blkidx[4] = {0, 0, 0, 1};

    unsigned char utmp[EVP_MAX_MD_SIZE];
    unsigned int utmp_len = 0;

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    if (!HMAC_Init_ex(&ctx, pass, pass_len, EVP_sha1(), NULL) ||
        !HMAC_Update(&ctx, salt, salt_len) ||
        !HMAC_Update(&ctx, blkidx, 4) ||
        !HMAC_Final(&ctx, utmp, &utmp_len)) {
        HMAC_CTX_cleanup(&ctx);
        return false;
    }
    memcpy(out, utmp, utmp_len);
    for (int j = 1; j < iter; j++) {
        // TODO: could we reuse ctx to get faster ?
        if (!HMAC(EVP_sha1(), (void *)pass, pass_len,
                  utmp, utmp_len, utmp, &utmp_len))
            return false;
        for (int k = 0; k < SHA1_LEN; k++)
            out[k] ^= utmp[k];
    }

    return true;
}
