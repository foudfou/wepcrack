#ifndef OPTIONS_H
#define OPTIONS_H

#include <limits.h>

struct opt_def {
    bool restore;
    char *wordlist;
};

extern struct opt_def options;

bool opt_parse(const int argc, char * const *argv);
void opt_clean(void);

#endif /* OPTIONS_H */
