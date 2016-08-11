/*
 * Check the validity of a WEP key, given all other necessary data.
 */
#ifndef WEP_H
#define WEP_H

#include <stdbool.h>

#define WEP_KEY_LEN 5
#define WEP_IV_LEN  3
#define WEP_ICV_LEN 4
#define WEP_PAYLOAD_MAX 256

struct wep_data_auth
{
    unsigned char frame[WEP_PAYLOAD_MAX]; /* plaintext */
    unsigned int  frame_len;
    unsigned char data[WEP_PAYLOAD_MAX]; /* including the ICV/CRC32 */
    unsigned int  data_len;
    unsigned char iv[WEP_IV_LEN];
};

bool wep_check_key_auth(const struct wep_data_auth *auth,
                        const unsigned char *key, unsigned int key_len);

bool wep_check_key_data(const unsigned char *key, unsigned int key_len);

#endif /* WEP_H */
