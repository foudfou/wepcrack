#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "config.h"
#include "dictionary.h"
#include "bit.h"
#include "ipc.h"
#include "options.h"

#define SEM_NAME_EMPTY "/wepcrack.emp"
#define SEM_NAME_FULL  "/wepcrack.ful"

int check_events(const int nprocs, const pid_t *pids)
{
    static int sigint = 0;
    if (BIT_CHK(events.sigs, EV_SIGUSR1)) {
        BIT_CLR(events.sigs, EV_SIGUSR1);
        sig_children(pids, nprocs, SIGUSR1);
    }
    if (BIT_CHK(events.sigs, EV_SIGINT)) {
        BIT_CLR(events.sigs, EV_SIGINT);
        sigint += 1;
        if (sigint > 1) {
            sig_children(pids, nprocs, SIGINT);
            return EV_NEXT_FIN;
        }
        fprintf(stderr, " Termination request."
                " Hit a second time to quit.\n");
    }
    return EV_NEXT_CONT;
}

static void
dict_consume(const struct dict_ctx *ctx, struct semphr sempair[2],
             const dict_apply_fn pw_apply)
{
    bool finish = false;
    for (;;) {
        // FIXME: make it a check_event()
        if (BIT_CHK(events.sigs, EV_SIGUSR1)) {
            BIT_CLR(events.sigs, EV_SIGUSR1);
        }
        if (BIT_CHK(events.sigs, EV_SIGUSR2)) {
            BIT_CLR(events.sigs, EV_SIGUSR2);
            if (events.sigpid == events.mainpid)
                finish = true;
        }
        if (BIT_CHK(events.sigs, EV_SIGINT)) {
            BIT_CLR(events.sigs, EV_SIGINT);
            return;
        }

        struct msg_buf msg;
        if (sem_trywait(sempair[1].semp) != 0) {
            if (errno == EAGAIN) {
                if (finish)
                    break;
            }
            else {
                perror("sem_trywait");
            }
            continue;
        }
        int ret = msg_get(ctx->msgqid, &msg, MSG_TYPE_WORD_READY);
        if (ret == MSG_GET_NOMSG) {
            fprintf(stderr, "ERROR: (%d) sem ok but no msg.\n", ctx->task_id);
        }
        else if (ret == -1) { // err
            fprintf(stderr, "ERROR: (%d) could not get msg.\n", ctx->task_id);
        }
        else if (ret > 0) {
            if (strlen(msg.text) == ctx->pw_len) {
                (*pw_apply)((unsigned char *)msg.text, ctx->pw_len);
            }
        }
        sem_post(sempair[0].semp);
    }
}

static bool
dict_fork(struct dict_ctx *ctx, pid_t *pids, struct semphr sempair[2],
          const dict_apply_fn pw_apply)
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

            ctx->task_id = i;
            fprintf(stderr, "%u (%u)\n", i, pids[i]);
            dict_consume(ctx, sempair, pw_apply);
            exit(EXIT_SUCCESS);
        }
    }
    return true;
}

// producer
bool dict_parse(struct dict_ctx *ctx, const dict_apply_fn pw_apply)
{
    struct semphr sempair[2] = {
        {0, SEM_NAME_EMPTY, ctx->nprocs},
        {0, SEM_NAME_FULL,  0},
    };
    if (!spair_init(sempair)) {
        fprintf(stderr, "sem open failed\n");
        return false;
    }

    pid_t pids[ctx->nprocs];
    if (!dict_fork(ctx, pids, sempair, pw_apply)) {
        fprintf(stderr, "fork failed\n");
        return false;
    }

    FILE * dictfile = fopen(options.wordlist, "r");
    if (!dictfile) {
        perror("fopen");
        return false;
    }

    off_t offset = -1;
    unsigned long long line = 0;
    char buf[MAX_LINE];
    while (fgets(buf, MAX_LINE, dictfile)) {
        if (check_events(ctx->nprocs, pids) == EV_NEXT_FIN)
            break;

        line += 1;
        const char *c = strchr(buf, '\n');
        if (!c) {
            fprintf(stderr, "WARNING: no '\\n' found line %llu.\n", line);
            continue;
        }

        ptrdiff_t idx = c - buf;
        offset += idx + 1;

        if (*(c - 1) == '\r')
            idx -= 1;

        struct msg_buf word_msg;
        word_msg.type = MSG_TYPE_WORD_READY;
        memcpy(word_msg.text, buf, idx);
        word_msg.text[idx] = '\0';
        sem_wait(sempair[0].semp);
        // FIXME: what if fails reagarding to sem_post() ?
        if (!msg_put(ctx->msgqid, &word_msg)) {
            fprintf(stderr, "ERROR: could queue word message: %s\n",
                    word_msg.text);
        }
        sem_post(sempair[1].semp);
    }

    if (ferror(dictfile))
        perror("input error");
    fclose(dictfile);

    sig_children(pids, ctx->nprocs, SIGUSR2);
    for (;;) {
        if (check_events(ctx->nprocs, pids) == EV_NEXT_FIN)
            break;

        int wstatus;
        pid_t wpid = waitpid(-1, &wstatus, WNOHANG);
        if (wpid == -1) {
            if (errno == ECHILD) {
                fprintf(stderr, "All processes ended.\n");
                break;
            }
            if (errno != EINTR) {
                perror("waitpid");
                /* gen_state_destroy(task_states_len, task_states); */
                return false;
            }
        }
    }

    spair_end(sempair);

    return true;
}
