/*
 * Generates all possible keys from a given alphabet with a cartesian power.
 * http://stackoverflow.com/a/23045070/421846
 */
#ifndef CARTESIAN_H
#define CARTESIAN_H

typedef void (* gen_apply_fn)(const unsigned char *, unsigned);

struct gen_ctx
{
    char                *alpha;
    unsigned            pw_len;
    unsigned            alpha_len;
    unsigned long long  total_n;
    int                 msgqid;
    long                msgid;
};

struct gen_ctx *gen_ctx_create(const char *a, const unsigned a_len,
                               const unsigned pw_len, const int qid);

void gen_ctx_destroy(struct gen_ctx *ctx);

void gen_apply_on_range(struct gen_ctx *ctx, gen_apply_fn fun,
                        unsigned long long from, unsigned long long until);

#endif /* CARTESIAN_H */
