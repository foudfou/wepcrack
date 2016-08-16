#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "generator.h"
#include "bit.h"
#include "utils.h"
#include "vars.h"

#define THRL_DELAY 5

/* Returns a new context, or NULL if any of the following occurs: malloc
 * failure, `total_n` overflow.
 * The context MUST be free'd with gen_ctx_destroy().
 */
struct gen_ctx *gen_ctx_create(const char *a, const unsigned a_len,
                               const unsigned pw_len)
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
    return ctx;
}

void gen_ctx_destroy(struct gen_ctx *ctx)
{
    free(ctx->alpha);
    free(ctx);
}

void gen_apply(struct gen_ctx *ctx, gen_apply_fn fun)
{
    unsigned char pw[ctx->pw_len];
    memset(pw, 0, ctx->pw_len);

    unsigned long long i, j;
    unsigned long long thrl = 0;
    alarm(THRL_DELAY);
    for (i = ctx->state.from; i < ctx->state.until; ++i) {
        if (BIT_CHK(events, EV_SIGUSR1)) {
            BIT_CLR(events, EV_SIGUSR1);
            fprintf(stderr, "(%d) currently at %llu (%llu keys/s).\n",
                    ctx->state.task_id, i, thrl);
        }
        if (BIT_CHK(events, EV_SIGINT)) {
            BIT_CLR(events, EV_SIGINT);
            fprintf(stderr, "(%d) saving state.\n", ctx->state.task_id);
            // TODO: save state and return;
        }
        if (BIT_CHK(events, EV_SIGALRM)) {
            BIT_CLR(events, EV_SIGALRM);
            thrl = (i - ctx->state.cur) / THRL_DELAY;
            ctx->state.cur = i;
            alarm(THRL_DELAY);
        }

        unsigned long long n = i;
        for (j = 0; j < ctx->pw_len; ++j){
            pw[ctx->pw_len -j -1] = ctx->alpha[n % ctx->alpha_len];
            n /= ctx->alpha_len;
        }

        (*fun)(pw, ctx->pw_len);
    }
}
