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
#include "utils.h"

#define SEM_NAME_EMPTY "/wepcrack.emp"
#define SEM_NAME_FULL  "/wepcrack.ful"

#define MAX_LINE_DICT_STATE 1024

#define DICT_STATE_SEP ":"
#define DICT_NSIGS 4

typedef int (*callback)(void *);

struct listener {
    callback cb;
    void     *data;
};

struct dict_state {
    unsigned long long line;
    unsigned long long line_prev;
    off_t              offset;
    unsigned long long thrl;
};

struct producer_cb_params {
    int                nprocs;
    pid_t             *pids;
    struct dict_state *state;
};

static int check_events(struct listener onsig[DICT_NSIGS]) {
    int evsig[DICT_NSIGS] = {EV_SIGUSR1, EV_SIGUSR2, EV_SIGINT, EV_SIGALRM};
    for (int i = 0; i < DICT_NSIGS; i++) {
        if (BIT_CHK(events.sigs, evsig[i])) {
            BIT_CLR(events.sigs, evsig[i]);
            if (onsig[i].cb) {
                if (onsig[i].cb(onsig[i].data) == EV_NEXT_FIN)
                    return EV_NEXT_FIN;
            }
        }
    }
    return EV_NEXT_CONT;
}


static int usr2_consumer_cb(void *finish) {
    if (events.sigpid == events.mainpid)
        *(bool*)finish = true;
    return EV_NEXT_CONT;
}

static int int_consumer_cb(void *data) {
    (void)data;
    return EV_NEXT_FIN;
}

static void
dict_consume(const struct dict_ctx *ctx, struct semphr sempair[2],
             const dict_apply_fn pw_apply)
{
    bool finish = false;

    struct listener onsignal[DICT_NSIGS] = {
        {NULL, NULL},
        {usr2_consumer_cb, (void*)&finish},
        {int_consumer_cb, NULL},
        {NULL, NULL},
    };

    for (;;) {
        if (check_events(onsignal) == EV_NEXT_FIN)
            break;

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

static bool dict_state_save(const struct dict_state *state)
{
    int ret = false;

    char statestr[MAX_LINE_DICT_STATE];
    char *abspath;
    abspath = realpath(options.wordlist, NULL);
    if (!abspath) {
        perror("realpath");
        return false;
    }
    else {
        snprintf(statestr, MAX_LINE_DICT_STATE,
                 "%s"DICT_STATE_SEP"%llu"DICT_STATE_SEP"%jd",
                 abspath, state->line, state->offset);
        free(abspath);
    }

    FILE * statefile = fopen(options.statefile, "w");
    if (!statefile) {
        perror("fopen");
        return ret;
    }

    if (fputs(statestr, statefile) == EOF) {
        fprintf(stderr, "fputs failed.\n");
        goto cleanup;
    }
    ret = true;

  cleanup:
    fclose(statefile);
    return ret;
}

#define DICT_STATE_READ_ECHK_STRTOK()  if (!tok) {                    \
    fprintf(stderr, "ERROR: premature ending when parsing state.\n"); \
    goto cleanup;                                                     \
  }

#define DICT_STATE_READ_ECHK_INTFROMSTR(msg)  if (err) {           \
    fprintf(stderr, "ERROR: error parsing %s from state.\n", msg); \
    goto cleanup;                                                  \
  }

static bool dict_state_read(struct dict_state *state)
{
    int ret = false;

    FILE * statefile = fopen(options.statefile, "r");
    if (!statefile) {
        perror("fopen");
        return false;
    }

    char buf[MAX_LINE];
    if (!fgets(buf, MAX_LINE, statefile)) {
        fprintf(stderr, "ERROR: could not read from in state file.\n");
        goto cleanup;
    }

    char *tok = strtok(buf, DICT_STATE_SEP);
    if (!tok)
        goto cleanup;
    options.wordlist = opt_stralloc(tok);

    bool err;
    tok = strtok(NULL, DICT_STATE_SEP);
    DICT_STATE_READ_ECHK_STRTOK()
    state->line = (intfromstr(tok, STRTOINT_ULL, &err)).ull;
    DICT_STATE_READ_ECHK_INTFROMSTR("line number")

    /* Couln't avoid code duplication: we could loop over an array of param
       structs. But offsetof() is not sufficient as we'd need to also pass the
       type of state member. Which we can't easily do without the non-portable
       typeof(), maybe with enum and switch, but that wouldn't be worth it. */
    tok = strtok(NULL, DICT_STATE_SEP);
    DICT_STATE_READ_ECHK_STRTOK();
    state->offset = (intfromstr(tok, STRTOINT_J, &err)).j;
    DICT_STATE_READ_ECHK_INTFROMSTR("offset");

    if (strtok(NULL, DICT_STATE_SEP)) {
        fprintf(stderr, "ERROR: remaining data in state file\n");
        goto cleanup;
    }

  cleanup:
    fclose(statefile);
    return ret;
}

static int usr1_producer_cb(void *data) {
    struct producer_cb_params *params = data;
    fprintf(stderr, "Currently at line %llu (%llu keys/s).\n",
            params->state->line, params->state->thrl);
    return EV_NEXT_CONT;
}

static int int_producer_cb(void *data) {
    struct producer_cb_params *params = data;
    static int sigint = 0;
    sigint += 1;
    if (sigint > 1) {
        sig_children(params->pids, params->nprocs, SIGINT);
        if (!dict_state_save(params->state))
            fprintf(stderr, "ERROR: Could not complete saving state.\n");
        else
            fprintf(stderr, "\nState saved to %s.\n", options.statefile);
        return EV_NEXT_FIN;
    }
    fprintf(stderr, " Termination request."
            " Hit a second time to quit.\n");
    return EV_NEXT_CONT;
}

static int alrm_producer_cb(void *data) {
    struct producer_cb_params *params = data;
    struct dict_state *state = params->state;
    state->thrl = (state->line - state->line_prev) / THRL_DELAY;
    state->line_prev = state->line;
    alarm(THRL_DELAY);
    return EV_NEXT_CONT;
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

    struct dict_state state = { 0 };
    struct producer_cb_params params = {ctx->nprocs, pids, &state};
    struct listener onsignal[DICT_NSIGS] = {
        {usr1_producer_cb, (void*)&params},
        {NULL, NULL},
        {int_producer_cb,  (void*)&params},
        {alrm_producer_cb, (void*)&params},
    };

    if (options.resume) {
        fprintf(stderr, "Restoring previous session from %s.\n", options.statefile);
        dict_state_read(&state);
        fprintf(stderr, "Resuming from %s, line %llu.\n", options.wordlist, state.line);
    }
    fprintf(stderr, "Parsing %s.\n", options.wordlist);

    FILE * dictfile = fopen(options.wordlist, "r");
    if (!dictfile) {
        perror("fopen");
        return false;
    }

    if (options.resume) {
        if (fseeko(dictfile, state.offset, SEEK_SET) != 0) {
            perror("fseek");
            fclose(dictfile);
            return false;
        }
    }

    alarm(THRL_DELAY);
    char buf[MAX_LINE];
    while (fgets(buf, MAX_LINE, dictfile)) {
        if (check_events(onsignal) == EV_NEXT_FIN)
            break;

        state.line += 1;
        state.offset = ftello(dictfile);
        const char *c = strchr(buf, '\n');
        if (!c) {
            fprintf(stderr, "WARNING: no '\\n' found line %llu.\n", state.line);
            continue;
        }

        ptrdiff_t idx = c - buf;
        if (idx >= MSG_TEXT_LEN) {
            fprintf(stderr, "WARNING: ignored too long password on line %llu.\n",
                    state.line);
            continue;
        }
        if (*(c - 1) == '\r')
            idx -= 1;

        struct msg_buf word_msg;
        word_msg.type = MSG_TYPE_WORD_READY;
        memcpy(word_msg.text, buf, idx);
        word_msg.text[idx] = '\0';
        if (sem_wait(sempair[0].semp) != 0) {
            perror("sem_wait");
            fprintf(stderr, "ERROR: sem_wait failed for word message: %s\n",
                    word_msg.text);
            continue;
        }
        if (!msg_put(ctx->msgqid, &word_msg)) {
            fprintf(stderr, "ERROR: could not queue word message: %s\n",
                    word_msg.text);
        }
        sem_post(sempair[1].semp);
    }

    if (ferror(dictfile))
        perror("input error");
    fclose(dictfile);

    fprintf(stderr, "Finished parsing wordlist.\n");
    sig_children(pids, ctx->nprocs, SIGUSR2);
    for (;;) {
        if (check_events(onsignal) == EV_NEXT_FIN)
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
                return false;
            }
        }
    }

    spair_end(sempair);

    return true;
}
