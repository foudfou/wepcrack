#include <string.h>
#include <openssl/hmac.h>
#include "crypto_ssl.h"
#include "xmpp.h"

bool xmpp_cli_proof(const char *pass, int pass_len,
                    const unsigned char *salt, int salt_len, int iter,
                    const char *auth_msg,
                    char *out, int out_max, int *out_len)
{
    unsigned char salt_bin[XMPP_PROOF_MAX] = { 0 };
    int salt_bin_len = 0;
    if (!ssl_base64(SSL_OP_DEC, (char *)salt_bin, XMPP_PROOF_MAX, &salt_bin_len,
                    salt, salt_len))
        return false;

    unsigned char salted_pass[SHA1_LEN] = { 0 };
    if (!ssl_pbkdf2_sha1(salted_pass, (unsigned char *)pass, pass_len,
                         salt_bin, salt_bin_len, iter))
        return false;

    const int cs_key_str_len = 10;
    const unsigned char cli_key_str[] = "Client Key";

    /* client_key = hmac_sha1(salted_pwd, "Client Key") */
    unsigned char cli_key[SHA1_LEN] = { 0 };
    unsigned int cli_key_len = 0;
    if (!HMAC(EVP_sha1(), (void *)salted_pass, SHA1_LEN,
              cli_key_str, cs_key_str_len, cli_key, &cli_key_len))
        return false;

    /* stored_key = hash_sha1(client_key) */
    unsigned char stored_key[EVP_MAX_MD_SIZE] = { 0 };
    int stored_key_len = 0;
    if (!ssl_digest(EVP_sha1(), stored_key, &stored_key_len,
                    cli_key, cli_key_len))
        return false;

    /* sig = hmac_sha1(stored_key, auth_msg) */
    unsigned char sig[SHA1_LEN] = { 0 };
    unsigned int sig_len = 0;
    if (!HMAC(EVP_sha1(), (void *)stored_key, stored_key_len,
              (unsigned char *)auth_msg, strlen(auth_msg), sig, &sig_len))
        return false;

    /* client_proof = b64_enc(xor_bytes(client_key, sig)) */
    for (int i = 0; i < SHA1_LEN; i++)
        cli_key[i] ^= sig[i];
    return ssl_base64(SSL_OP_ENC, out, out_max, out_len, cli_key, cli_key_len);
}

bool xmpp_auth_check(const char *pass, int pass_len,
                     const unsigned char *salt, int salt_len, int iter,
                     const char *auth_msg, const char *against)
{
    char out[XMPP_PROOF_MAX] = { 0 };
    int out_len = 0;
    bool rv = xmpp_cli_proof(pass, pass_len, salt, salt_len, iter, auth_msg,
                             out, XMPP_PROOF_MAX, &out_len);

    return (rv && memcmp(out, against, out_len) == 0);

}
