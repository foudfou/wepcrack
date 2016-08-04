// http://stackoverflow.com/a/23045070/421846

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


unsigned long long powull(unsigned long long base, unsigned long long exp){
    unsigned long long result = 1;
    while (exp > 0) {
        if (exp & 1)
            result *= base;
        base = base * base;
        exp >>=1;
    }
    return result;
}

struct gen_ctx
{
    char               *alpha;
    unsigned            pw_len;
    unsigned            alpha_len;
    unsigned long long  total_n;
};

struct gen_ctx *gen_ctx_create(const char *a, const unsigned len) {
    struct gen_ctx *ctx = malloc(sizeof(struct gen_ctx));
    ctx->alpha_len = strlen(a);
    ctx->alpha = calloc(ctx->alpha_len, sizeof(char));
    strncpy(ctx->alpha, a, ctx->alpha_len);
    ctx->pw_len = len;
    ctx->total_n = powull(ctx->alpha_len, ctx->pw_len);
}

void gen_ctx_destroy(struct gen_ctx *ctx) {
    free(ctx->alpha);
    free(ctx);
}

void gen_with_range(struct gen_ctx *ctx,
                    unsigned long long start, unsigned long long end) {
    char pw[ctx->pw_len+1];
    memset(pw, 0, ctx->pw_len+1);
    unsigned long long i, j;
    for (i = start; i < end; ++i){
        unsigned long long n = i;
        for (j = 0; j < ctx->pw_len; ++j){
            pw[ctx->pw_len -j -1] = ctx->alpha[n % ctx->alpha_len];
            n /= ctx->alpha_len;
        }
        printf("[%s]\n", pw);
    }
}


int main(int argc, char *argv[]){
    struct gen_ctx *ctx = gen_ctx_create("abcdef", 4);

    gen_with_range(ctx, 0, ctx->total_n);

    gen_ctx_destroy(ctx);

    return 0;
}
