/*
 * Generates all possible keys from a given alphabet with a cartesian power.
 * http://stackoverflow.com/a/23045070/421846
 */
#ifndef GENERATOR_H
#define GENERATOR_H

typedef void (* gen_apply_fn)(const unsigned char *, unsigned);

struct gen_task_state
{
    int                task_id;
    unsigned long long from;
    unsigned long long until;
    unsigned long long cur;
};

struct gen_ctx
{
    char                  *alpha;
    unsigned              pw_len;
    unsigned              alpha_len;
    unsigned long long    total_n;
    struct gen_task_state state;
};

struct gen_ctx *gen_ctx_create(const char *a, const unsigned a_len,
                               const unsigned pw_len);

void gen_ctx_destroy(struct gen_ctx *ctx);

void gen_apply(struct gen_ctx *ctx, gen_apply_fn fun);

#endif /* GENERATOR_H */
