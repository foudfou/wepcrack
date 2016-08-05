// http://stackoverflow.com/a/23045070/421846

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include "crypto_ssl.h"

/* #define ALPHABET "abcdef" */
/* #define ALPHABET_LEN 6 */
/* #define PASSWORD_LEN 4 */

#define ALPHABET                                                   \
"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f" \
"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f" \
"\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f" \
"\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f" \
"\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f" \
"\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f" \
"\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f" \
"\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f" \
"\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f" \
"\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f" \
"\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf" \
"\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf" \
"\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf" \
"\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf" \
"\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef" \
"\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff"
#define ALPHABET_LEN 256
#define PASSWORD_LEN 2


typedef void (* gen_apply_fn)(const unsigned char *, unsigned);

struct gen_ctx
{
    char                *alpha;
    unsigned            pw_len;
    unsigned            alpha_len;
    unsigned long long  total_n;
};


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

void tohex(char *dst, const unsigned char *src, size_t len)
{
    for (size_t i = 0; i < len; i++)
        snprintf(&(dst[2*i]), 3, "%02x", src[i]);
    dst[2*len+1] = 0;
}

void print_hex(const unsigned char *bytes, unsigned len) {
    char dst[2*len+1];
    tohex(dst, bytes, len);
    printf("%s\n", dst);
}

struct gen_ctx *gen_ctx_create(const char *a, const unsigned a_len,
                               const unsigned pw_len) {
    struct gen_ctx *ctx = malloc(sizeof(struct gen_ctx));
    ctx->alpha_len = a_len;
    ctx->alpha = calloc(ctx->alpha_len, sizeof(char));
    memcpy(ctx->alpha, a, ctx->alpha_len);
    ctx->pw_len = pw_len;
    ctx->total_n = powull(ctx->alpha_len, ctx->pw_len);
    return ctx;
}

void gen_ctx_destroy(struct gen_ctx *ctx) {
    free(ctx->alpha);
    free(ctx);
}

void gen_apply_on_range(struct gen_ctx *ctx, gen_apply_fn fun,
                        unsigned long long from, unsigned long long until) {
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


int main(void) {
    struct gen_ctx *crack_ctx =
        gen_ctx_create(ALPHABET, ALPHABET_LEN, PASSWORD_LEN);
    gen_apply_fn pw_apply = print_hex;

#pragma omp parallel firstprivate(crack_ctx)
    {
        int ithread = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        unsigned long long from = crack_ctx->total_n*ithread/nthreads;
        unsigned long long until = crack_ctx->total_n*(ithread + 1)/nthreads;
        fprintf(stderr, "%u: %lli -> %lli\n", ithread, from, until);
        gen_apply_on_range(crack_ctx, pw_apply, from, until);
    }

    gen_ctx_destroy(crack_ctx);

    return 0;
}


/* iv + key -(RC4)-> keystream
 * keystream ^ challenge -> data */
unsigned char iv[] = "\x52\x61\x82";
unsigned char challenge[] =
    "\xe9\xad\xe5\xd8\x36\x51\x7a\x60\x3b\x42\x6b\x13\x44\x57\x60\x7a"
    "\x48\x31\x82\xcc\x3d\x69\x46\x8f\x93\x82\xff\x06\xde\x43\x24\xc8"
    "\xf0\x09\xa0\x26\x5b\x1b\x86\x96\x5d\xf1\xa9\xa2\x49\x0a\x1c\x91"
    "\x3b\x9f\x5d\x78\x08\xa3\x08\x9b\x26\x07\xa1\x04\x4a\xc6\xcc\x3a"
    "\xcf\x6d\x60\x2a\x88\xe6\xc0\xe5\xd8\x6a\x87\x21\x74\xa4\xb2\xaf"
    "\x43\x0f\x27\x4b\xb3\x2f\xe7\xd9\x36\x88\xdd\x80\x4e\xaa\xba\x1e"
    "\x17\x1a\x48\x9f\x01\x09\x84\xd9\x73\x0c\xfa\xe7\xb0\xac\x96\xf3"
    "\xbb\xbd\x3e\x6e\xed\x25\x47\x23\xae\x25\xa4\xfc\xcf\x5e\x1a\xe6";
unsigned char data[] =
    "\x45\xbe\x60\x0e\xd0\x67\x83\xe0\xe1\x80\x13\x30\x79\x23\x89\x3a"
    "\xb2\x74\xb2\xd2\x38\x44\x52\xb3\xc3\xcb\x59\x93\x44\xd0\x20\xb3"
    "\x9b\xf9\x6c\xe4\x8f\x6e\x2f\xb1\xa5\x1d\xb0\x19\x89\xb6\x4c\x53"
    "\x6c\xdb\xc6\xc1\xfb\x60\xf2\x31\xff\xe9\xf2\x74\xc7\x59\x46\xda"
    "\xb8\x78\x63\x86\xe4\x63\xbc\x00\x9f\xee\x31\xf4\xf7\x51\x96\xe5"
    "\x75\xef\x3b\xeb\xaf\x7b\xcf\xa7\xda\x0d\x4a\x56\x75\x10\x39\x9a"
    "\x0a\xa6\x9e\x9a\x59\x31\x40\x0c\x27\xb5\x22\x65\x39\xdc\x53\x85"
    "\x15\xc7\x7f\xfa\xd9\x02\x14\xd5\x9a\x44\xcc\xf2\x43\xde\x9c\x98"
    "\x1f\xd6\x5b\x79\xe8\xbb\x52\xb1";
unsigned char icv[]= "\x49\x20\x50\x35"; /* CRC32 of payload */
