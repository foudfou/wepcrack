#include <stdlib.h>
#include <string.h>
#include "cartesian.h"
#include "utils.h"

struct gen_ctx *gen_ctx_create(const char *a, const unsigned a_len,
                               const unsigned pw_len)
{
    struct gen_ctx *ctx = malloc(sizeof(struct gen_ctx));
    ctx->alpha_len = a_len;
    ctx->alpha = calloc(ctx->alpha_len, sizeof(char));
    memcpy(ctx->alpha, a, ctx->alpha_len);
    ctx->pw_len = pw_len;
    ctx->total_n = powull(ctx->alpha_len, ctx->pw_len);
    return ctx;
}

void gen_ctx_destroy(struct gen_ctx *ctx)
{
    free(ctx->alpha);
    free(ctx);
}

void gen_apply_on_range(struct gen_ctx *ctx, gen_apply_fn fun,
                        unsigned long long from, unsigned long long until)
{
    unsigned char pw[ctx->pw_len];
    memset(pw, 0, ctx->pw_len);

    unsigned long long i, j;
    for (i = from; i < until; ++i){
        unsigned long long n = i;
        for (j = 0; j < ctx->pw_len; ++j){
            pw[ctx->pw_len -j -1] = ctx->alpha[n % ctx->alpha_len];
            n /= ctx->alpha_len;
        }
        (*fun)(pw, ctx->pw_len);
    }
}
