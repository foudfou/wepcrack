// http://stackoverflow.com/a/23045070/421846

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define ALPHA_MAX 256


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
    char               alpha[ALPHA_MAX];
    unsigned           pw_len;
    unsigned           alpha_len;
    unsigned long long total_n;
};

void gen_ctx_init(struct gen_ctx *ctx, const char *a, const unsigned len) {
    strncpy(ctx->alpha, a, ALPHA_MAX);
    ctx->alpha_len = strlen(ctx->alpha);
    ctx->pw_len = len;
    ctx->total_n = powull(ctx->alpha_len, ctx->pw_len);
}

void gen_with_range(struct gen_ctx *ctx,
                    unsigned long long start, unsigned long long end) {
    char *pw = calloc(ctx->pw_len+1, sizeof(char));
    unsigned long long i, j;
    for (i = start; i < end; ++i){
        unsigned long long n = i;
        for (j = 0; j < ctx->pw_len; ++j){
            pw[ctx->pw_len -j -1] = ctx->alpha[n % ctx->alpha_len];
            n /= ctx->alpha_len;
        }
        printf("[%s]\n", pw);
    }
    free(pw);
}


int main(int argc, char *argv[]){
    struct gen_ctx ctx;
    gen_ctx_init(&ctx, "abcdef", 4);
    gen_with_range(&ctx, 0, ctx.total_n);

    return 0;
}
