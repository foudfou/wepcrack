#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

struct opt_def options = {
    .restore = false,
    .wordlist = 0,
    .statefile = "/tmp/wepcrack.dmp",
};

static struct option opt_long[] = {
    {"restore",   no_argument,       0, 'r'},
    {"wordlist",  required_argument, 0, 'w'},
    {"statefile", required_argument, 0, 's'},
    { 0 },
};
static char *opt_str = "hrw:s:";

static void usage()
{
  printf("Usage: wepcrack [ options ]\n");
  printf("  -h, --help\n");
  printf("  -r, --restore\n");
  printf("  -w, --wordlist DICTFILE\n");
  printf("  -s, --statefile STATEFILE for storing dumped state\n");
}

static bool opt_check(void)
{
    if (options.restore && options.wordlist) {
        fprintf(stderr, "ERROR: -r and -w can't be used together.\n");
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
        case 'r':
            options.restore = true;
            break;
        case 'w':
            options.wordlist = opt_stralloc(optarg);
            if (!options.wordlist)
                return 1;
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
