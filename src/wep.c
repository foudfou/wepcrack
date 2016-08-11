#include <assert.h>
#include <string.h>
#include "crc32.h"
#include "crypto_ssl.h"
#include "utils.h"
#include "wep.h"

/* (iv + key = key for RC4/keystream) ^ (frame header + challenge + icv/crc32)
 * -> data.  This is exactly what the openssl RC4() function does: xor's some
 * input with the RC4 PRNG derived from a key.
 */
bool wep_check_key_auth(const struct wep_data_auth *auth,
                        const unsigned char *key, unsigned int key_len) {
    assert(key_len == WEP_KEY_LEN);

    // compute key (IV+key)
    unsigned int ivkey_len = WEP_IV_LEN + key_len;
    unsigned char ivkey[ivkey_len];
    memcpy(ivkey, auth->iv, WEP_IV_LEN);
    memcpy(ivkey + WEP_IV_LEN, key, key_len);
    print_hex(ivkey, ivkey_len);

    // compute ICV of plain data
    uint32_t crc_raw = crc32(auth->frame, auth->frame_len);
    unsigned char crc[WEP_ICV_LEN];
    *(uint32_t *)crc = crc_raw;
    print_hex(crc, WEP_ICV_LEN);

    // compute whole plain text (frame + ICV)
    unsigned int frameicv_len = auth->frame_len + WEP_ICV_LEN;
    unsigned char frameicv[frameicv_len];
    memcpy(frameicv, auth->frame, auth->frame_len);
    memcpy(frameicv + auth->frame_len, crc, WEP_ICV_LEN);
    print_hex(frameicv, frameicv_len);

    unsigned char out[256];
    int out_len = 0;
    unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 };
    int encrypt = 1;
    bool rv = ssl_crypt(EVP_rc4(), out, &out_len, frameicv, frameicv_len,
                        ivkey, ivkey_len, iv, encrypt);
    assert((unsigned int)out_len == frameicv_len);
    assert(frameicv_len == auth->data_len);

    fprintf(stderr, "rc4 (%s) -> %u\n", BOOL2STR(rv), out_len);
    print_hex(out, out_len);

    return (!memcmp(out, auth->data, auth->data_len));
}

// TODO: ...and this is why we'll be only working on data
bool wep_check_key_data(const unsigned char *key, unsigned int key_len) {
    assert(key_len == WEP_KEY_LEN);

    /* def isValidKey(self,key,wepdatapkt): */
      /*   c=ARC4.new(wepdatapkt[Dot11WEP].iv+key) */
      /*   dataICV=c.decrypt(str(wepdatapkt)[-4-len(wepdatapkt.wepdata):]) */
      /*   data=dataICV[:-4] */
      /*   icv=dataICV[-4:] */
      /*   if crc32(data) in struct.unpack('<l',icv): */
      /*     return True */
      /*   else: */
      /*     return False */

    return true;
}
