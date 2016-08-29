#ifndef OPTIONS_H
#define OPTIONS_H

#include <limits.h>

struct opts {
    bool restore;
    char *wordlist;
};

extern struct opts options;

bool args_parse(const int argc, char * const *argv);

#endif /* OPTIONS_H */
