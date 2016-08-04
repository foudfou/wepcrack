#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// http://stackoverflow.com/a/13370778/421846

/* The alphabet consists of bytes [0,255] */
static const unsigned char UPPER = 9; // UCHAR_MAX;
static const unsigned char TAKE = 3;

void print_pw(unsigned char *s, int ithr)
{
    printf("%u: ", ithr);
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

    long long glob_iter =
        (long long)pow((double)(UPPER + 1), (double)TAKE);
    printf("total glob_iter:  %lli\n", glob_iter);
    long long k;

#pragma omp parallel private(pw, k) firstprivate(carry)
    {
        long long thr_iter = 0;

        // pw needs to be initialized here like
        const int nthreads = omp_get_num_threads();
        const int ithread = omp_get_thread_num();
        const long long start = glob_iter * ithread / nthreads;
        const long long finish = glob_iter * (ithread + 1) / nthreads;

        for (int r = 0; r < TAKE; r++) {
            pw[r] = start % (long long)pow((double)(UPPER + 1), (double)(TAKE-r));
        }

#pragma omp for schedule(monotonic:static)
        for (long long k = start; k < finish; k++) {
            ++thr_iter;

#pragma omp critical
            print_pw(pw, ithread);

            if (pw[TAKE-1] + 1 > UPPER) {
                pw[TAKE-1] = 0;
                carry[TAKE-2] = true;
            }
            else {
                pw[TAKE-1] += 1;
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
                if (pw[0] + 1 > UPPER) {  /* FIXME: limit should be finish */
                    printf("\nAccomplisshed!\n");
                    /* break; */
                    #pragma omp cancel for
                }
                else {
                    pw[0] += 1;
                }
                carry[0] = false;
            }

#pragma omp cancellation point for
        }

        printf("Thread %u: %lli iterations completed\n", ithread, thr_iter);

    } // end omp parallel

    return 0;
}
