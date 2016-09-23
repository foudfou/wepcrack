#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stdbool.h>

typedef void (* dict_apply_fn)(const unsigned char *, unsigned);

struct dict_state {
    unsigned long long  line;
    off_t               offset;
};

struct producer_cb_params {
    int                 nprocs;
    pid_t              *pids;
    struct dict_state  *state;
};

struct dict_ctx
{
    unsigned pw_len;
    int      msgqid;
    int      nprocs;
    int      task_id;
};

bool dict_parse(struct dict_ctx *ctx, const dict_apply_fn pw_apply);

#endif /* DICTIONARY_H */
