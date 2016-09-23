#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bit.h"
#include "ipc.h"
#include "options.h"
#include "utils.h"
#include "wep.h"
#include "generator.h"

#define GEN_STATE_SEP ":"
#define THRL_DELAY 5

/* Returns a new context, or NULL if any of the following occurs: malloc
 * failure, `total_n` overflow.
 * The context MUST be free'd with gen_ctx_destroy().
 */
struct gen_ctx *gen_ctx_create(const char *a, const unsigned a_len,
                               const unsigned pw_len, const int qid)
{
    struct gen_ctx *ctx = malloc(sizeof(struct gen_ctx));
    if (!ctx) {
        perror("malloc");
        return NULL;
    }
    ctx->alpha_len = a_len;
    ctx->alpha = calloc(ctx->alpha_len, sizeof(char));
    memcpy(ctx->alpha, a, ctx->alpha_len);
    ctx->pw_len = pw_len;
    ctx->total_n = powull(ctx->alpha_len, ctx->pw_len);
    if (!ctx->total_n) {
        fprintf(stderr, "Space too big %u^%u (>%llu).\n", a_len, pw_len,
                ULLONG_MAX);
        return NULL;
    }
    ctx->msgqid = qid;

    ctx->nprocs = 0;
    ctx->state.task_id = 0;
    ctx->state.from = 0;
    ctx->state.until = 0;
    ctx->state.cur = 0;

    return ctx;
}

void gen_ctx_destroy(struct gen_ctx *ctx)
{
    free(ctx->alpha);
    free(ctx);
}

void gen_apply(struct gen_ctx *ctx, const gen_apply_fn fun)
{
    unsigned char pw[ctx->pw_len];
    memset(pw, 0, ctx->pw_len);

    unsigned long long state_last = ctx->state.cur;
    unsigned long long thrl = 0;
    alarm(THRL_DELAY);
    while (ctx->state.cur < ctx->state.until) {
        if (BIT_CHK(events.sigs, EV_SIGUSR1)) {
            BIT_CLR(events.sigs, EV_SIGUSR1);
            fprintf(stderr, "(%d) currently at %llu (%llu keys/s).\n",
                    ctx->state.task_id, ctx->state.cur, thrl);
        }
        if (BIT_CHK(events.sigs, EV_SIGINT)) {
            BIT_CLR(events.sigs, EV_SIGINT);
            struct msg_buf state_msg;
            state_msg.type = MSG_TYPE_TASK_STATE;
            // serialize
            snprintf(state_msg.text, MSG_TEXT_LEN, "%d"GEN_STATE_SEP
                     "%llu"GEN_STATE_SEP"%llu"GEN_STATE_SEP"%llu",
                     ctx->state.task_id, ctx->state.from, ctx->state.until,
                     ctx->state.cur);
            if (!msg_put(ctx->msgqid, &state_msg)) {
                fprintf(stderr, "ERROR: could not send state to parent: %s\n",
                        state_msg.text);
            }
            return;
        }
        if (BIT_CHK(events.sigs, EV_SIGALRM)) {
            BIT_CLR(events.sigs, EV_SIGALRM);
            thrl = (ctx->state.cur - state_last) / THRL_DELAY;
            state_last = ctx->state.cur;
            alarm(THRL_DELAY);
        }

        unsigned long long n = ctx->state.cur;
        unsigned long long j;
        for (j = 0; j < ctx->pw_len; ++j){
            pw[ctx->pw_len -j -1] = ctx->alpha[n % ctx->alpha_len];
            n /= ctx->alpha_len;
        }

        (*fun)(pw, ctx->pw_len);

        ctx->state.cur += 1;
    }
}

int gen_state_save(const struct gen_ctx *ctx)
{
    int ret = 0;

    FILE * statefile = fopen(options.statefile, "w");
    if (!statefile) {
        perror("fopen");
        return ret;
    }

    int msg_count = 0;
    // TODO: add timeout
    struct msg_buf msg;
    while (msg_count < ctx->nprocs) {
        if (msg_get_sync(ctx->msgqid, &msg, 0) > 0) {
            msg_count += 1;
            if (fputs(msg.text, statefile) == EOF) {
                fprintf(stderr, "fputs failed.\n");
                goto cleanup;
            }
            if (fputc('\n', statefile) == EOF) {
                fprintf(stderr, "fputc failed.\n");
                goto cleanup;
            }
        }
    }
    ret = msg_count;

  cleanup:
    fclose(statefile);
    return ret;
}

/* Return length of states read, or -1 on error.
 * The allocated `struct gen_task_state` array MUST be free'd with
 * gen_state_destroy().
 */
int gen_state_read(struct gen_task_state *states[MAX_PROCS])
{
    int len = 0;

    FILE * statefile = fopen(options.statefile, "r");
    if (!statefile) {
        perror("fopen");
        return len;
    }

    unsigned int line = 0;
    char buf[MAX_LINE];
    while (fgets(buf, MAX_LINE, statefile)) {
        line = len + 1;

        // deserialize
        char * taskstr = strtok(buf, GEN_STATE_SEP);
        if (!taskstr) {
            len = -1;
            goto cleanup;
        }
        bool err;

        struct gen_task_state *state = malloc(sizeof(struct gen_task_state));
        union int_t task = intfromstr(taskstr, STRTOINT_L, &err);
        state->task_id = task.l;
        if (err) {
            fprintf(stderr, "ERROR: parsing error on line %d\n", line);
            len = -1;
            goto cleanup;
        }

        unsigned long long *statem[3] = {
            &state->from, &state->until, &state->cur };
        for (int i = 0; i < 3; ++i) {
            char *str = strtok(NULL, GEN_STATE_SEP);
            *statem[i] = (intfromstr(str, STRTOINT_ULL, &err)).ull;
            if (err) {
                fprintf(stderr, "ERROR: parsing error on line %d\n", line);
                len = -1;
                goto cleanup;
            }
        }

        if (strtok(NULL, GEN_STATE_SEP)) {
            fprintf(stderr, "ERROR: remaining data on line %d\n", line);
            len = -1;
            goto cleanup;
        }

        len += 1;
        states[len-1] = state;
    }

    if (ferror(statefile))
        perror("input error");

  cleanup:
    fclose(statefile);
    return len;
}

void gen_state_destroy(int states_len, struct gen_task_state *states[MAX_PROCS])
{
    for (int i = 0; i < states_len; ++i)
        free(states[i]);
}

static bool
gen_fork(pid_t *pids, struct gen_ctx *ctx, const gen_apply_fn pw_apply,
         const int states_len, struct gen_task_state * const states[MAX_PROCS])
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

bool gen_deploy(struct gen_ctx *ctx, const int nprocs, const gen_apply_fn pw_apply)
{
    if (!ctx) {
        fprintf(stderr, "Can't create context. Exiting.\n");
        return false;
    }

    int task_states_len = 0;
    struct gen_task_state *task_states[MAX_PROCS] = { 0 };
    if (options.resume) {
        fprintf(stderr, "Restoring from %s.\n", options.statefile);
        ctx->nprocs = task_states_len = gen_state_read(task_states);
        if (!ctx->nprocs) {
            fprintf(stderr, "No task states found. Exiting.\n");
            return false;
        }
        if (ctx->nprocs > nprocs) {
            fprintf(stderr, "More tasks than available cpus. Exiting.\n");
            gen_state_destroy(task_states_len, task_states);
            return false;
        }
        if (task_states_len < nprocs) {
            fprintf(stderr, "WARNING: Less tasks found than available cpus."
                    " No extra redistribution will be applied.\n");
        }
    }
    else {
        fprintf(stderr, "Generating all possibilities.\n");
        ctx->nprocs = nprocs;
    }

    pid_t pids[nprocs];
    if (!gen_fork(pids, ctx, pw_apply, task_states_len, task_states)) {
        fprintf(stderr, "fork failed\n");
        gen_state_destroy(task_states_len, task_states);
        return false;
    }

    int sigint = 0;
    for (;;) {
        if (BIT_CHK(events.sigs, EV_SIGUSR1)) {
            BIT_CLR(events.sigs, EV_SIGUSR1);
            sig_children(pids, ctx->nprocs, SIGUSR1);
        }
        if (BIT_CHK(events.sigs, EV_SIGINT)) {
            BIT_CLR(events.sigs, EV_SIGINT);
            sigint += 1;
            if (sigint > 1) {
                sig_children(pids, ctx->nprocs, SIGINT);
                if (gen_state_save(ctx) != ctx->nprocs) {
                    fprintf(stderr, "ERROR: Could not complete saving state.\n");
                    gen_state_destroy(task_states_len, task_states);
                    return false;
                }
                fprintf(stderr, "\nState saved to %s.\n", options.statefile);
                break;
            }
            fprintf(stderr, " Termination request."
                    " Hit a second time to quit.\n");
        }

        int wstatus;
        pid_t wpid = waitpid(-1, &wstatus, WNOHANG);
        if (wpid == -1) {
            if (errno == ECHILD) {
                fprintf(stderr, "All processes ended.\n");
                break;
            }
            if (errno != EINTR) {
                perror("waitpid");
                gen_state_destroy(task_states_len, task_states);
                return false;
            }
        }
    }

    return true;
}
