#ifndef WEP_DEFS_H
#define WEP_DEFS_H


#define WEP_KEY_LEN 5
#define WEP_IV_LEN  3
#define WEP_ICV_LEN 4

struct wep_data_auth
{
    unsigned char chall[256];   /* plaintext */
    unsigned int  chall_len;
    unsigned char data[256];
    unsigned int  data_len;
    unsigned char iv[WEP_IV_LEN];
    unsigned char icv[WEP_ICV_LEN]; /* CRC32 of payload */
};


#endif /* WEP_DEFS_H */
