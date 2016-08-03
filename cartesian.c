#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// http://stackoverflow.com/a/13370778/421846

/* The alphabet consists of bytes [0,255] */
static const unsigned char UPPER = UCHAR_MAX;
static const unsigned char TAKE = 3;

void print_pw(unsigned char *s)
{
    for (int i = 0; i < TAKE; i++){
        printf("%02x ", s[i]);
    }
    printf("\n");
}


int main(int argc, char **argv)
{
    unsigned char pw[TAKE];
    bool carry[TAKE];
    for (int r = 0; r < TAKE; r++) {
        pw[r] = 0;
        carry[r] = 0;
    }

    for (;;) {

        print_pw(pw);

        if (pw[TAKE-1] + 1 > UPPER) {
            pw[TAKE-1] = 0;
            carry[TAKE-2] = true;
        }
        else {
            pw[TAKE - 1] += 1;
        }

        for (int i = TAKE - 2; i > 0; i--) {
            if (carry[i]) {
                if (pw[i] + 1 > UPPER) {
                    pw[i] = 0;
                    carry[i-1] = true;
                }
                else {
                    pw[i] += 1;
                }
                carry[i] = false;
            }
        }

        if (carry[0]) {
            if (pw[0] + 1 > UPPER) {
                printf("\nAccomplisshed!\n");
                break;
            }
            else {
                pw[0] += 1;
            }
            carry[0] = false;
        }

    }

    return 0;
}
