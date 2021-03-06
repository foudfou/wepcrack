/*
 * Generates all possible keys from a given alphabet with a cartesian power.
 * http://stackoverflow.com/a/23045070/421846
 */
#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdbool.h>
#include "config.h"

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
    int                   msgqid;
    int                   nprocs;
    struct gen_task_state state;
};

struct gen_ctx *gen_ctx_create(const char *a, const unsigned a_len,
                               const unsigned pw_len, const int qid);

void gen_ctx_destroy(struct gen_ctx *ctx);

void gen_apply(struct gen_ctx *ctx, const gen_apply_fn fun);

int gen_state_save(const struct gen_ctx *ctx);
int gen_state_read(struct gen_task_state *states[MAX_PROCS]);
void gen_state_destroy(int states_len, struct gen_task_state *states[MAX_PROCS]);

bool gen_deploy(struct gen_ctx *ctx, const int nprocs,
                const gen_apply_fn pw_apply);

#endif /* GENERATOR_H */
