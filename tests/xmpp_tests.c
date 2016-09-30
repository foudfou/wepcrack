#include <string.h>
#include <openssl/hmac.h>
#include "test.h"
#include "xmpp.h"
#include "xmpp_data.h"

int main(void)
{
    char proof[XMPP_PROOF_MAX];
    int proof_len = 0;
    bool rv = xmpp_cli_proof(xmpp_pwd2, strlen(xmpp_pwd2),
                             xmpp_salt2, xmpp_salt2_len,
                             xmpp_iter2,
                             xmpp_auth_msg2,
                             proof, XMPP_PROOF_MAX, &proof_len);
    ASSERT(rv && memcmp(proof, xmpp_cli_final_proof2, proof_len) == 0)

    rv = xmpp_auth_check(xmpp_pwd2, strlen(xmpp_pwd2),
                         xmpp_salt2, xmpp_salt2_len,
                         xmpp_iter2, xmpp_auth_msg2, xmpp_cli_final_proof2);
    ASSERT(rv)

    return 0;
}
