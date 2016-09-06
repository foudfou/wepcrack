// http://stackoverflow.com/a/23045070/421846

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bit.h"
#include "generator.h"
#include "ipc.h"
#include "options.h"
#include "utils.h"
#include "vars.h"
#include "wep.h"
#include "wep_data.h"

// FIXME: all `const` declarations need to be checked: int const * const

void sig_handler(int signo)
{
    switch (signo) {
    case SIGALRM:
        BIT_SET(events, EV_SIGALRM);
        break;
    case SIGUSR1:
        BIT_SET(events, EV_SIGUSR1);
        break;
    case SIGTERM: /* ^\ */
    case SIGQUIT: /* ^\ */
    case SIGHUP:
    case SIGINT:  /* ^C */
        BIT_SET(events, EV_SIGINT);
        break;
    default:
        fprintf(stderr, "caught unhandled %d\n", signo);
    }
}

bool sig_install()
{
    int sig[][2] = {
        {SIGALRM, SA_RESTART},
        {SIGUSR1, SA_RESTART},
        {SIGINT,  SA_RESTART},
        {SIGHUP,  SA_RESTART},
        {SIGQUIT, SA_RESTART},
        {SIGTERM, SA_RESTART}
    };
    int len = sizeof(sig) / sizeof(int) / 2;

    struct sigaction sa;
    sa.sa_handler = sig_handler;
    for (int i = 0; i < len; i++) {
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = sig[i][1];
        if (sigaction(sig[i][0], &sa, NULL) != 0) {
            perror("sigaction");
            return false;
        }
    }

    return true;
}

void sig_children(const pid_t *pids, const int nprocs, const int sig)
{
    for (int i = 0; i < nprocs; i++)
        kill(pids[i], sig);
}

int msg_install(const char *path)
{
   int qid = msg_qid(path);
   if (qid != -1) {
       fprintf(stderr, "Found existing message queue. Cleaning...\n");
       struct msg_buf _;
       while (msg_get(qid, &_, 0) > 0);
   }
   return msg_create(path);
}

static void wep_check_key_with_data(const unsigned char *key, unsigned len)
{
    if (wep_check_key_auth(&wep_auth1, key, len)) {
        char keyhex[2*WEP_KEY_LEN+1];
        tohex(keyhex, key, WEP_KEY_LEN);
        printf("!!! KEY FOUND -> 0x%s !!!\n", keyhex);
    }
}

bool gen_fork(pid_t *pids, struct gen_ctx *ctx, const gen_apply_fn pw_apply,
              const int states_len,
              struct gen_task_state * const states[MAX_PROCS])
{
    for (int i = 0; i < ctx->nprocs; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return false;
        }

        if (pids[i] == 0) {
            // we don't want terminal SIGINT (^C) to be sent to children
            if (setpgid(0, 0) == -1) {
                perror("setpgid");
                return false;
            }

            if (states_len) {
                ctx->state.task_id = states[i]->task_id;
                ctx->state.from = states[i]->from;
                ctx->state.until = states[i]->until;
                ctx->state.cur = states[i]->cur;
            }
            else {
                ctx->state.task_id = i;
                ctx->state.from = ctx->total_n*i/ctx->nprocs;
                ctx->state.until = ctx->total_n*(i + 1)/ctx->nprocs;
                ctx->state.cur = ctx->state.from;
            }

            fprintf(stderr, "%u: %llu -> %llu (%llu)\n", ctx->state.task_id,
                    ctx->state.from, ctx->state.until, ctx->state.cur);
            gen_apply(ctx, pw_apply);
            exit(EXIT_SUCCESS);
        }
    }
    return true;
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
    if (options.wordlist) {
        fprintf(stderr, "Dict=%s\n", options.wordlist);
    }

    int nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    pid_t pids[nprocs];

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

    struct gen_ctx *crack_ctx =
        gen_ctx_create(WEP_ALPHABET, WEP_ALPHABET_LEN, WEP_KEY_LEN, qid);
    if (!crack_ctx) {
        fprintf(stderr, "Can't create context. Exiting.\n");
        retcode = EXIT_FAILURE;
        goto cleanup_l0;
    }
    gen_apply_fn pw_apply = wep_check_key_with_data;

    int task_states_len = 0;
    struct gen_task_state *task_states[MAX_PROCS] = { 0 };
    if (options.restore) {
        fprintf(stderr, "Restoring from %s.\n", options.statefile);
        crack_ctx->nprocs = task_states_len = gen_state_read(task_states);
        if (!crack_ctx->nprocs) {
            fprintf(stderr, "No task states found. Exiting.\n");
            retcode = EXIT_FAILURE;
            goto cleanup_l1;
        }
        if (crack_ctx->nprocs > nprocs) {
            fprintf(stderr, "More tasks than available cpus. Exiting.\n");
            retcode = EXIT_FAILURE;
            goto cleanup_l1;
        }
        if (task_states_len < nprocs) {
            fprintf(stderr, "WARNING: Less tasks found than available cpus."
                    " No extra redistribution will be applied.\n");
        }
    }
    else {
        fprintf(stderr, "Generating all possibilities.\n");
        crack_ctx->nprocs = nprocs;
    }

    if (!gen_fork(pids, crack_ctx, pw_apply, task_states_len, task_states)) {
        fprintf(stderr, "fork failed\n");
        retcode = EXIT_FAILURE;
        goto cleanup_l1;
    }

    pid_t wpid;
    int wstatus;
    int sigint = 0;
    for (;;) {
        if (BIT_CHK(events, EV_SIGUSR1)) {
            BIT_CLR(events, EV_SIGUSR1);
            sig_children(pids, crack_ctx->nprocs, SIGUSR1);
        }
        if (BIT_CHK(events, EV_SIGINT)) {
            BIT_CLR(events, EV_SIGINT);
            sigint += 1;
            if (sigint > 1) {
                sig_children(pids, crack_ctx->nprocs, SIGINT);
                int ret = gen_state_save(qid, crack_ctx->nprocs);
                if (ret != crack_ctx->nprocs) {
                    fprintf(stderr, "ERROR: Could not complete saving state.\n");
                    retcode = EXIT_FAILURE;
                    goto cleanup_l1;
                }
                fprintf(stderr, "\nState saved to %s.\n", options.statefile);
                break;
            }
            fprintf(stderr, " Termination request."
                    " Hit a second time to quit.\n");
        }

        wpid = waitpid(-1, &wstatus, WNOHANG);
        if (wpid == -1) {
            if (errno == ECHILD) {
                fprintf(stderr, "All proceseses ended.\n");
                break;
            }
            if (errno != EINTR) {
                perror("waitpid");
                retcode = EXIT_FAILURE;
                goto cleanup_l1;
            }
        }
    }

    fprintf(stderr,"Bye.\n");

  cleanup_l1:
    gen_state_destroy(task_states_len, task_states);
  cleanup_l0:
    gen_ctx_destroy(crack_ctx);
    msg_destroy(qid);
    opt_clean();

    return retcode;
}

/* This will be used for wordlist/passworddict parsing
    if (options.wordlist) {
        FILE * dictfile = fopen(options.wordlist, "r");
        if (!dictfile) {
            perror("fopen");
            retcode = EXIT_FAILURE;
            goto cleanup;
        }

        off_t offset = -1;
        unsigned long long line = 0;
        char buf[MAX_LINE];
        char pw[MAX_LINE-1];
        while (fgets(buf, MAX_LINE, dictfile)) {
            line += 1;
            const char *c = strchr(buf, '\n');
            if (c) {
                ptrdiff_t idx = c - buf;
                offset += idx + 1;
                printf("%lu, %lu: %s", idx, offset, buf);
            }
            else {
                fprintf(stderr, "WARNING: no \\n found line %llu.\n", line);
            }
        }

        if (ferror(dictfile))
            perror("input error");

        fclose(dictfile);
        goto cleanup;
    }
*/
