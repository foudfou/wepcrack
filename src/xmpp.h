#ifndef XMPP_H
#define XMPP_H

#include <stdbool.h>

#define XMPP_PROOF_MAX 512

bool xmpp_cli_proof(const char *pass, int pass_len,
                    const unsigned char *salt, int salt_len, int iter,
                    const char *auth_msg,
                    char *out, int out_max, int *out_len);

bool xmpp_auth_check(const char *pass, int pass_len,
                     const unsigned char *salt, int salt_len, int iter,
                     const char *auth_msg, const char *against);

#endif /* XMPP_H */
