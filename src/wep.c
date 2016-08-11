#include <assert.h>
#include <string.h>
#include "crc32.h"
#include "crypto_ssl.h"
#include "utils.h"
#include "wep.h"

static void compute_ivkey(unsigned char *ivkey, const unsigned char *iv,
                       const unsigned char *key, const unsigned int key_len) {
    memcpy(ivkey, iv, WEP_IV_LEN);
    memcpy(ivkey + WEP_IV_LEN, key, key_len);
}

static void compute_crc(unsigned char *crc,
                     const unsigned char *frame, const unsigned int frame_len) {
    uint32_t crc_raw = crc32(frame, frame_len);
    *(uint32_t *)crc = crc_raw;
}

/* (iv + key = key for RC4/keystream) ^ (frame header + challenge + icv/crc32)
 * -> data.  This is exactly what the openssl RC4() function does: xor's some
 * input with the RC4 PRNG derived from a key.
 */
bool wep_check_key_auth(const struct wep_data *auth,
                        const unsigned char *key, unsigned int key_len) {
    assert(key_len == WEP_KEY_LEN);

    unsigned int ivkey_len = WEP_IV_LEN + key_len;
    unsigned char ivkey[ivkey_len];
    compute_ivkey(ivkey, auth->iv, key, key_len);
    print_hex(ivkey, ivkey_len);

    unsigned char crc[WEP_ICV_LEN];
    compute_crc(crc, auth->frame, auth->frame_len);
    print_hex(crc, WEP_ICV_LEN);

    // compute whole plain text (frame + ICV)
    unsigned int frameicv_len = auth->frame_len + WEP_ICV_LEN;
    unsigned char frameicv[frameicv_len];
    memcpy(frameicv, auth->frame, auth->frame_len);
    memcpy(frameicv + auth->frame_len, crc, WEP_ICV_LEN);
    print_hex(frameicv, frameicv_len);

    unsigned char out[WEP_PAYLOAD_MAX+WEP_ICV_LEN];
    int out_len = 0;
    unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 };
    int encrypt = 1;
    bool crypt_ok = ssl_crypt(EVP_rc4(), out, &out_len, frameicv, frameicv_len,
                              ivkey, ivkey_len, iv, encrypt);
    assert(crypt_ok);
    assert((unsigned int)out_len == frameicv_len);
    assert(frameicv_len == auth->data_len);

    fprintf(stderr, "rc4 (%s) -> %u\n", BOOL2STR(crypt_ok), out_len);
    print_hex(out, out_len);

    return (!memcmp(out, auth->data, auth->data_len));
}

bool wep_check_key_data(const struct wep_data *auth,
                        const unsigned char *key, unsigned int key_len) {
    assert(key_len == WEP_KEY_LEN);

    unsigned int ivkey_len = WEP_IV_LEN + key_len;
    unsigned char ivkey[ivkey_len];
    compute_ivkey(ivkey, auth->iv, key, key_len);

    unsigned char out[WEP_PAYLOAD_MAX+WEP_ICV_LEN];
    int out_len = 0;
    unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 };
    int encrypt = 0;
    bool decrypt_ok = ssl_crypt(EVP_rc4(), out, &out_len, auth->data, auth->data_len,
                                ivkey, ivkey_len, iv, encrypt);
    assert(decrypt_ok);
    assert((unsigned int)out_len == auth->data_len);

    unsigned int data_len = out_len - WEP_ICV_LEN;
    unsigned char crc[WEP_ICV_LEN];
    compute_crc(crc, out, data_len);
    print_hex(crc, WEP_ICV_LEN);

    unsigned char icv[WEP_ICV_LEN];
    memcpy(icv, out + data_len, WEP_ICV_LEN);
    print_hex(icv, WEP_ICV_LEN);

    return (!memcmp(crc, icv, WEP_ICV_LEN));
}
