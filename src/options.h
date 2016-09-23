#ifndef OPTIONS_H
#define OPTIONS_H

#include <limits.h>

struct opt_def {
    bool resume;
    bool dictionary;
    char *wordlist;
    char *statefile;
};

extern struct opt_def options;

char *opt_stralloc(const char *optarg);
int opt_parse(const int argc, char * const *argv);
void opt_clean(void);

#endif /* OPTIONS_H */
