#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "dict.h"
#include "ipc.h"
#include "options.h"

bool dict_parse(const int qid)
{
    FILE * dictfile = fopen(options.wordlist, "r");
    if (!dictfile) {
        perror("fopen");
        return false;
    }

    off_t offset = -1;
    unsigned long long line = 0;
    char buf[MAX_LINE];
    while (fgets(buf, MAX_LINE, dictfile) && (line < 20)) { /* TESTING */
        line += 1;
        const char *c = strchr(buf, '\n');
        if (c) {
            ptrdiff_t idx = c - buf;
            offset += idx + 1;

            struct msg_buf word_msg;
            word_msg.type = MSG_TYPE_WORD_READY;
            memcpy(word_msg.text, buf, idx);
            word_msg.text[idx] = '\0';
            if (!msg_put(qid, &word_msg)) {
                fprintf(stderr, "ERROR: could queue word message: %s\n",
                        word_msg.text);
            }
        }
        else {
            fprintf(stderr, "WARNING: no \\n found line %llu.\n", line);
        }
    }

    if (ferror(dictfile))
        perror("input error");

  cleanup:
    fclose(dictfile);
    return true;
}
