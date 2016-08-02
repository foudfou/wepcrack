// http://stackoverflow.com/a/13370778/421846
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~

struct context
{
    char *alpha;
    unsigned int alpha_len;
    int take;
};

void print_pw(unsigned int *s, struct context *ctx);


int main(int argc, char **argv)
{
    struct context ctx;
    /* char ALPHA[] = "ABCDEF"; */
    ctx.alpha = 0;
    ctx.alpha_len = 256;        // UCHAR_MAX limits.h
    ctx.take = 3;

    /* Type must be large enough to contain alphabet + 1. Otherwise, we run
       into overflow issues. */
    unsigned int pw[ctx.take];
    for (int r = 0; r < ctx.take; r++) pw[r] = 0;

    print_pw(pw, &ctx);

    for (;;) {
        int end = 0;
        for (int k = 0; k < ctx.take; k++)
            if (pw[k] == ctx.alpha_len - 1) end++;
        if (end == ctx.take) {
            print_pw(pw, &ctx);
            printf("\nAccomplisshed!\n");
            break;
        }

        pw[ctx.take-1] += 1;

        for (int i = ctx.take - 1; i > 0; i--) {
            if (pw[i] == ctx.alpha_len) {
                pw[i] = 0;
                pw[i-1] += 1;
            }
        }

        print_pw(pw, &ctx);
    }

    return 0;
}


/* This only print. */
void print_pw(unsigned int *s, struct context *ctx)
{
    for (int i = 0; i < ctx->take; i++){
        printf("%02x ", s[i]);
    }
    printf("\n");
}
