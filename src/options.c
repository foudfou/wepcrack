#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

struct opt_def options = {
    .restore = false,
    .wordlist = 0,
};

static struct option opt_long[] = {
    {"restore",  no_argument,       0, 'r'},
    {"wordlist", required_argument, 0, 'w'},
    { 0 },
};
static char *opt_str = "hrw:";

static void usage()
{
  printf("Usage: wepcrack [ options ]\n");
  printf("  -h, --help\n");
  printf("  -r, --restore\n");
  printf("  -w, --wordlist DICTFILE\n");
}

static bool opt_check(void)
{
    if (options.restore && options.wordlist) {
        fprintf(stderr, "ERROR: -r and -w can't be used together.\n");
        return false;
    }

    return true;
}

bool opt_parse(const int argc, char * const *argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int ret = 0, cont = 1;

    do {
        ret = getopt_long(argc, argv, opt_str, opt_long, 0);
        switch (ret) {
        case '?':
            usage();
            return false;
        case -1:
            cont = 0;
            break;
        case 'h':
            usage();
            break;
        case 'r':
            options.restore = true;
            break;
        case 'w':
            options.wordlist = (char *)malloc(strlen(optarg) + 1);
            if (!options.wordlist) {
                perror("malloc");
                return false;
            }
            strcpy(options.wordlist, optarg);
            break;
        }
    } while (cont);

    /* remaining command line arguments (not options). */
    if (optind < argc) {
        // TODO:
    }

    if (!opt_check())
        return false;

    return true;
}

void opt_clean(void)
{
    if (options.wordlist)
        free(options.wordlist);
}
