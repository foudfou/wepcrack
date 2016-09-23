#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

struct opt_def options = {
    .dictionary = false,
    .resume = false,
    .wordlist = 0,
    .statefile = "/tmp/wepcrack.dmp",
};

/* best vaoid `optional_argument` which requires no space btw. the switch and
 * the value (-oarg) or '       = ' (--opt=arg)... */
static struct option opt_long[] = {
    {"dictionary", required_argument, 0, 'D'},
    {"resume",    no_argument,       0, 'r'},
    {"wordlist",   required_argument, 0, 'w'},
    {"statefile",  required_argument, 0, 's'},
    { 0 },
};
static char *opt_str = "Dhrw:s:";

static void usage()
{
  printf("Usage: wepcrack [options...]\n");
  printf("  -h, --help\n");
  printf("  -D, --dictionary, parse from wordlist instead of generating\n");
  printf("  -r, --resume\n");
  printf("  -w, --wordlist DICTFILE, implies --dictionary\n");
  printf("  -s, --statefile STATEFILE, for storing dumped state\n");
}

static bool opt_check(void)
{
    if (options.resume && options.wordlist) {
        fprintf(stderr, "ERROR: -r and -w can't be used together.\n");
        return false;
    }
    if (options.dictionary && !options.wordlist && !options.resume) {
        fprintf(stderr, "ERROR: -D requires -w.\n");
        return false;
    }

    return true;
}

char * opt_stralloc(const char *optarg)
{
    char *str = malloc(strlen(optarg) + 1);
    if (str)
        strcpy(str, optarg);
    else
        perror("malloc");
    return str;
}

/* Returns
 * - 0 on success and the caller needs to continue execution
 * - -1 on success and the caller needs to halt further execution
 * - 1 on error  */
int opt_parse(const int argc, char * const *argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int ret = 0, cont = 1;

    do {
        ret = getopt_long(argc, argv, opt_str, opt_long, 0);
        switch (ret) {
        case '?':
            usage();
            return 1;
        case -1:
            cont = 0;
            break;
        case 'h':
            usage();
            return -1;
            break;
        case 'D':
            options.dictionary = true;
            break;
        case 'r':
            options.resume = true;
            break;
        case 'w':
            options.wordlist = opt_stralloc(optarg);
            if (!options.wordlist)
                return 1;
            options.dictionary = true;
            break;
        case 's':
            options.statefile = opt_stralloc(optarg);
            if (!options.statefile)
                return 1;
            break;
        }
    } while (cont);

    /* remaining command line arguments (not options). */
    if (optind < argc) {
        // TODO:
    }

    if (!opt_check())
        return 1;

    return 0;
}

void opt_clean(void)
{
    if (options.wordlist)
        free(options.wordlist);
}
