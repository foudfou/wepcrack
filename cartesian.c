// http://stackoverflow.com/a/13370778/421846
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~

struct context
{
    char *alpha;
    int alpha_len;
    int take;
};

void all(char *s, struct context *ctx);
void reverse_print(char *s, struct context *ctx);


int main(int argc, char **argv)
{
    struct context ctx;
    ctx.alpha = argv[1];
    ctx.alpha_len = strlen(ctx.alpha);
    ctx.take = atoi(argv[2]);
    char pw[ctx.take];           // char as numbers
    for (int r = 0; r < ctx.take; r++) pw[r] = 0;

    reverse_print(pw, &ctx);
    all(pw, &ctx);

    return 0;
}

void all(char *s, struct context *ctx)
{
    /* Basic Case */
    int end = 0;
    for (int k = ctx->take - 1; k >= 0; k--)
        if (s[k] == ctx->alpha_len - 1) end++;
    if (end == ctx->take) {
        reverse_print(s, ctx);
        printf("\nAccomplisshed!\n");
        return;
    }

    s[0] += 1;
    for (int i = 0; i < ctx->take - 1; i++) {
        if (s[i] == ctx->alpha_len) {
            s[i] = 0;
            s[i+1] += 1;
        }
    }
    reverse_print(s, ctx);
    all(s, ctx);
}

/* This only prints. */
void reverse_print(char *s, struct context *ctx)
{
    for(int i = ctx->take - 1; i >= 0; i--){
        printf("%c", (ctx->alpha[s[i]]));
    }
    printf(" ");
}
