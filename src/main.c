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
            return(false);
        }
    }

    return(true);
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
       while (msg_get_sync(qid, &_, 0) > 0);
   }
   else {
       qid = msg_create(path);
       if (qid == -1) {
           fprintf(stderr, "Could not create message queue. Exiting...\n");
           return(EXIT_FAILURE);
       }
   }
   return qid;
}

int state_save(const int qid, const int nprocs)
{
    int msg_count = 0;
    // TODO: add timeout
    struct msg_buf msg;
    while (msg_count < nprocs) {
        if (msg_get_sync(qid, &msg, 0) > 0) {
            msg_count += 1;
            fprintf(stderr, "main got msg: %s\n", msg.text);
        }
    }
    return 0;
}

static void wep_check_key_with_data(const unsigned char *key, unsigned len)
{
    if (wep_check_key_auth(&wep_auth1, key, len)) {
        char keyhex[2*WEP_KEY_LEN+1];
        tohex(keyhex, key, WEP_KEY_LEN);
        printf("!!! KEY FOUND -> 0x%s !!!\n", keyhex);
    }
}


/* TODO:
  + add session save/restore
 */
int main(int argc, char *argv[])
{
    if (!opt_parse(argc, argv)) {
        fprintf(stderr, "Argument error. Exiting.\n");
        return(EXIT_FAILURE);
    }
    if (options.restore) {
        fprintf(stderr, "Restoring...\n");
    }
    if (options.wordlist) {
        fprintf(stderr, "Dict=%s\n", options.wordlist);
    }
    return(EXIT_SUCCESS);

    int nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    pid_t pids[nprocs];

    /* We install the handler before forking, so the children inherit it. */
    sig_install();
    /* The msg queue will be used to pass children's state to the parent. */
    int qid = msg_install(argv[0]);

    struct gen_ctx *crack_ctx =
        gen_ctx_create(WEP_ALPHABET, WEP_ALPHABET_LEN, WEP_KEY_LEN, qid);
    if (!crack_ctx) {
        fprintf(stderr, "Can't create context. Exiting.\n");
        return(EXIT_FAILURE);
    }
    gen_apply_fn pw_apply = wep_check_key_with_data;

    for (int i = 0; i < nprocs; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return(EXIT_FAILURE);
        }
        if (pids[i] == 0) {
            // we don't want terminal SIGINT (^C) to be sent to children
            if (setpgid(0, 0) == -1) {
                perror("setpgid");
                return(EXIT_FAILURE);
            }
            crack_ctx->state.task_id = i;
            crack_ctx->state.from = crack_ctx->total_n*i/nprocs;
            crack_ctx->state.until = crack_ctx->total_n*(i + 1)/nprocs;
            crack_ctx->state.cur = crack_ctx->state.from;
            fprintf(stderr, "%u: %lli -> %lli\n", i, crack_ctx->state.from,
                    crack_ctx->state.until);
            gen_apply(crack_ctx, pw_apply);
            return(EXIT_SUCCESS);
        }
    }

    pid_t wpid;
    int wstatus;
    int sigint = 0;
    for (;;) {
        if (BIT_CHK(events, EV_SIGUSR1)) {
            BIT_CLR(events, EV_SIGUSR1);
            sig_children(pids, nprocs, SIGUSR1);
        }
        if (BIT_CHK(events, EV_SIGINT)) {
            BIT_CLR(events, EV_SIGINT);
            sigint += 1;
            if (sigint > 1) {
                sig_children(pids, nprocs, SIGINT);
                state_save(qid, nprocs);
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
                return(EXIT_FAILURE);
            }
        }
    }

    fprintf(stderr,"Main done!\n");

    msg_destroy(crack_ctx->msgqid);
    gen_ctx_destroy(crack_ctx);
    opt_clean();

    return(EXIT_SUCCESS);
}
