// http://stackoverflow.com/a/23045070/421846

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "generator.h"
#include "dictionary.h"
#include "ipc.h"
#include "options.h"
#include "utils.h"
#include "xmpp.h"
#include "xmpp_data.h"

// FIXME: all `const` declarations need to be checked: int const * const

static void xmpp_check_auth_with(const unsigned char *key, unsigned len)
{
    if (xmpp_auth_check((char *)key, len, xmpp_salt1, xmpp_salt1_len,
                        xmpp_iter1, xmpp_auth_msg1, xmpp_cli_final_proof1)) {
        char keyhex[2*len];
        tohex(keyhex, key, len);
        printf("!!! KEY FOUND -> %s (0x%s) !!!\n", key, keyhex);
    }
}


int main(int argc, char *argv[])
{
    int retcode = EXIT_SUCCESS;

    int ret = opt_parse(argc, argv);
    if (ret > 0) {
        fprintf(stderr, "Argument error. Exiting.\n");
        return EXIT_FAILURE;
    }
    else if (ret < 0) {
        return EXIT_SUCCESS;
    }

    /* We install the handler before forking, so the children inherit it. */
    if (!sig_install()) {
        fprintf(stderr, "Could not install signals. Exiting...\n");
        return EXIT_FAILURE;
    }

    /* The msg queue will be used to pass children's state to the parent, or
     * passwords to children. */
    int qid = msg_install(argv[0]);
    if (qid == -1) {
        fprintf(stderr, "Could not create message queue. Exiting...\n");
        return EXIT_FAILURE;
    }

    int nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    events.mainpid = getpid();

    dict_apply_fn pw_apply = xmpp_check_auth_with;
    if (options.dictionary) {
        struct dict_ctx ctx = {
            // FIXME: pw_len needs to be config
            .pw_len = 0, .msgqid = qid, .nprocs = nprocs,
            .task_id = -1
        };
        if (!dict_parse(&ctx, pw_apply))
            retcode = EXIT_FAILURE;
    }
    else {
        struct gen_ctx *ctx =
            // FIXME: alpha and aplha_len need to be config
            gen_ctx_create(0, 0, 0, qid);
        if (!gen_deploy(ctx, nprocs, pw_apply))
            retcode = EXIT_FAILURE;
    }

    fprintf(stderr,"Bye.\n");

    msg_destroy(qid);
    opt_clean();

    return retcode;
}
