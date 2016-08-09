#ifndef WEP_AUTH_H
#define WEP_AUTH_H

#include <assert.h>
#include <openssl/evp.h>
#include "utils.h"
#include "crc32.h"
#include "wep_defs.h"


/* (iv + key = key for RC4/keystream) ^ (challenge + icv/crc32) -> data
 * This is exactly what the openssl RC4() function does.
 */
void wep_check_key_auth(const struct wep_data_auth *auth,
                        const unsigned char *key, unsigned int key_len) {
    assert(key_len == WEP_KEY_LEN);

    // compute key (IV+key)
    unsigned int ivkey_len = WEP_IV_LEN + key_len;
    unsigned char ivkey[ivkey_len];
    memcpy(ivkey, auth->iv, WEP_IV_LEN);
    memcpy(ivkey + WEP_IV_LEN, key, key_len);
    print_hex(ivkey, ivkey_len);

    // compute ICV
    uint32_t crc_raw = crc32(auth->chall, auth->chall_len);
    unsigned char crc[WEP_ICV_LEN];
    *(uint32_t *)crc = crc_raw;
    print_hex(crc, WEP_ICV_LEN);

    // compute whole plain text (chall + ICV)
    unsigned int challicv_len = auth->chall_len + WEP_ICV_LEN;
    unsigned char challicv[challicv_len];
    memcpy(challicv, auth->chall, auth->chall_len);
    memcpy(challicv + auth->chall_len, crc, WEP_ICV_LEN);
    print_hex(challicv, challicv_len);

    unsigned char out[256];
    int out_len = 0;
    unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 };
    int encrypt = 1;
    bool rv = ssl_crypt(EVP_rc4(), out, &out_len, challicv, challicv_len,
                        ivkey, ivkey_len, iv, encrypt);
    fprintf(stderr, "rc4 (%s) -> %u\n", rv ? "true" : "false", out_len);

    print_hex(out, out_len);

    /* TODO: to be continued... For now, I can't seem to recreate the correct
       encrypted data from the challenge. Moreover, I can't find an explanation
       as to why the data is always 4 bytes+ longer than the challenge... */
}

// TODO: ...and this is why we'll be only working on data
void wep_check_key_data(const unsigned char *key, unsigned int key_len) {
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
}


#endif /* WEP_AUTH_H */
