/*
 * Check the validity of a WEP key, given all other necessary data.
 */
#ifndef WEP_H
#define WEP_H

#define WEP_KEY_LEN 5
#define WEP_IV_LEN  3
#define WEP_ICV_LEN 4

struct wep_data_auth
{
    unsigned char frame[256];   /* plaintext */
    unsigned int  frame_len;
    unsigned char data[256];
    unsigned int  data_len;
    unsigned char iv[WEP_IV_LEN];
    unsigned char icv[WEP_ICV_LEN]; /* CRC32 of payload */
};

void wep_check_key_auth(const struct wep_data_auth *auth,
                        const unsigned char *key, unsigned int key_len);

void wep_check_key_data(const unsigned char *key, unsigned int key_len);

#endif /* WEP_H */
