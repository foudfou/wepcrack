#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// http://stackoverflow.com/a/13370778/421846

/* The alphabet consists of bytes [0,255] */
static const unsigned int UPPER = UCHAR_MAX;
static const unsigned int TAKE = 3;

void print_pw(unsigned int *s)
{
    for (int i = 0; i < TAKE; i++){
        printf("%02x ", s[i]);
    }
    printf("\n");
}


int main(int argc, char **argv)
{
    /* Type must be large enough to contain alphabet + 1. Otherwise, we run
       into overflow issues. */
    unsigned int pw[TAKE];
    for (int r = 0; r < TAKE; r++) pw[r] = 0;

    print_pw(pw);

    for (;;) {
        int end = 0;
        for (int k = 0; k < TAKE; k++)
            if (pw[k] == UPPER) end++;
        if (end == TAKE) {
            print_pw(pw);
            printf("\nAccomplisshed!\n");
            break;
        }

        pw[TAKE-1] += 1;

        for (int i = TAKE - 1; i > 0; i--) {
            if (pw[i] == UPPER + 1) {
                pw[i] = 0;
                pw[i-1] += 1;
            }
        }

        print_pw(pw);
    }

    return 0;
}
