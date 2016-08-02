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

void reverse_print(char *s, struct context *ctx);


int main(int argc, char **argv)
{
    struct context ctx;
    ctx.alpha = "ABCDEF";
    ctx.alpha_len = strlen(ctx.alpha);
    ctx.take = 4;

    char pw[ctx.take];           // char as numbers
    for (int r = 0; r < ctx.take; r++) pw[r] = 0;

    reverse_print(pw, &ctx);

    for (;;) {
        int end = 0;
        for (int k = ctx.take - 1; k >= 0; k--)
            if (pw[k] == ctx.alpha_len - 1) end++;
        if (end == ctx.take) {
            reverse_print(pw, &ctx);
            printf("\nAccomplisshed!\n");
            break;
        }

        pw[0] += 1;
        for (int i = 0; i < ctx.take - 1; i++) {
            if (pw[i] == ctx.alpha_len) {
                pw[i] = 0;
                pw[i+1] += 1;
            }
        }
        reverse_print(pw, &ctx);
    }

    return 0;
}

/* This only prints. */
void reverse_print(char *s, struct context *ctx)
{
    for(int i = ctx->take - 1; i >= 0; i--){
        printf("%c", (ctx->alpha[s[i]]));
    }
    printf(" ");
}
