#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include "options.h"

struct opts options = {
    .restore = false,
    .wordlist = 0,
};

static struct option opts_long[] = {
    {"restore",  no_argument,       0, 'r'},
    {"wordlist", required_argument, 0, 'w'},
    { 0 },
};
static char *opts_def = "hrw:";

static void usage()
{
  printf("Usage: wepcrack [ options ]\n");
  printf("  -h, --help\n");
  printf("  -r, --restore\n");
  printf("  -w, --wordlist DICTFILE\n");
}

bool args_parse(const int argc, char * const *argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int ret = 0, cont = 1;

    do {
        ret = getopt_long(argc, argv, opts_def, opts_long, 0);
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
            options.wordlist = optarg;
            break;
        }
    } while (cont);

    /* remaining command line arguments (not options). */
    if (optind < argc) {
        // TODO:
    }


    return true;
}
