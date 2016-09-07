// http://stackoverflow.com/a/23045070/421846

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "generator.h"
#include "dict.h"
#include "ipc.h"
#include "options.h"
#include "utils.h"
#include "wep.h"
#include "wep_data.h"

// FIXME: all `const` declarations need to be checked: int const * const

static void wep_check_key_with_data(const unsigned char *key, unsigned len)
{
    if (wep_check_key_auth(&wep_auth1, key, len)) {
        char keyhex[2*WEP_KEY_LEN+1];
        tohex(keyhex, key, WEP_KEY_LEN);
        printf("!!! KEY FOUND -> 0x%s !!!\n", keyhex);
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

    if (options.wordlist) {
        fprintf(stderr, "Parsing %s.\n", options.wordlist);
        // TODO: dict_fork();
        dict_parse(qid);
    }
    else {
        gen_apply_fn pw_apply = wep_check_key_with_data;
        retcode = gen_deploy(qid, nprocs, pw_apply) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    fprintf(stderr,"Bye.\n");

    msg_destroy(qid);
    opt_clean();

    return retcode;
}
