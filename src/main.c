// http://stackoverflow.com/a/23045070/421846

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bit.h"
#include "cartesian.h"
#include "ipc.h"
#include "utils.h"
#include "wep.h"
#include "wep_data.h"

/* #define ALPHABET "abcdef" */
/* #define ALPHABET_LEN 6 */
/* #define PASSWORD_LEN 4 */

#define ALPHABET                                                   \
"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f" \
"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f" \
"\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f" \
"\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f" \
"\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f" \
"\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f" \
"\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f" \
"\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f" \
"\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f" \
"\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f" \
"\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf" \
"\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf" \
"\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf" \
"\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf" \
"\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef" \
"\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff"
#define ALPHABET_LEN 256

#define EV_SIGUSR1 (1U << 0)

static unsigned int events = 0;

void sig_handler(int signo)
{
    switch (signo) {
    case SIGUSR1:
        fprintf(stderr, "(%u) got SIGUSR1\n", getpid());
        fflush (stderr);
        BIT_SET(events, EV_SIGUSR1);
        break;
    case SIGUSR2:
        fprintf(stderr, "(%u) got SIGUSR2\n", getpid());
        fflush (stderr);
        break;
    case SIGINT :
        fprintf(stderr, "(%u) got SIGINT\n", getpid());
        fflush(stderr);
        break;
    }
}

int sig_install(int sig)
{
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(sig, &sa, NULL) != 0) {
        perror("sigaction");
        return(EXIT_FAILURE);
    }
    return(EXIT_SUCCESS);
}

void msg_children(const long nprocs, const int qid)
{
    struct msgbuf msg;
    for (int i = 1; i <= nprocs; i++) {
        msg.type =  i;
        snprintf(msg.text, MSG_TEXT_LEN, "hi%d", i);
        msg_put(qid, &msg);
    }
}

static void wep_check_key_with_data(const unsigned char *key, unsigned len)
{
    if (wep_check_key_auth(&wep_auth1, key, len)) {
        char keyhex[2*WEP_KEY_LEN+1];
        tohex(keyhex, key, WEP_KEY_LEN);
        printf("!!! KEY FOUND -> 0x%s !!!\n", keyhex);
    }
}

/* TODO:
  + add session save/restore
  + add throttle speed
 */
int main(int argc, char *argv[])
{
    (void)argc;  // unused

    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    pid_t pid[nprocs+1];

    sig_install(SIGUSR1);
    sig_install(SIGUSR2);

    int qid = msg_qid(argv[0]);
    if (qid != -1) {
        fprintf(stderr, "Found existing message queue. Cleaning...\n");
        msg_destroy(qid);
    }
    qid = msg_create(argv[0]);
    if (qid == -1) {
        fprintf(stderr, "Could not create message queue. Exiting...\n");
        return(EXIT_FAILURE);
    }

    struct gen_ctx *crack_ctx =
        gen_ctx_create(ALPHABET, ALPHABET_LEN, WEP_KEY_LEN, qid);

    gen_apply_fn pw_apply = wep_check_key_with_data;
    for (int i = 1; i <= nprocs; i++) {
        pid[i] = fork();
        if (pid[i] == -1) {
            perror("fork");
            return(EXIT_FAILURE);
        }
        if (pid[i] == 0) {
            crack_ctx->msgid = i;
            unsigned long long from = crack_ctx->total_n*i/nprocs;
            unsigned long long until = crack_ctx->total_n*(i + 1)/nprocs;
            fprintf(stderr, "%u: %lli -> %lli\n", i, from, until);
            gen_apply_on_range(crack_ctx, pw_apply, from, until);
            return(EXIT_SUCCESS);
        }
    }

    pid_t wpid;
    int wstatus;
    do {
        if (BIT_CHK(events, EV_SIGUSR1)) {
            BIT_CLR(events, EV_SIGUSR1);
            fprintf(stderr, "(%u) fwd SIGUSR1 to children\n", getpid());
            msg_children(nprocs, crack_ctx->msgqid);
        }

        wpid = waitpid(0, &wstatus, WNOHANG | WUNTRACED | WCONTINUED);
        if (wpid == -1 && errno != EINTR) {
            perror("waitpid");
            return(EXIT_FAILURE);
        }

        if (WIFEXITED(wstatus)) {
            printf("exited, status=%d\n", WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
            printf("killed by signal %d\n", WTERMSIG(wstatus));
        } else if (WIFSTOPPED(wstatus)) {
            printf("stopped by signal %d\n", WSTOPSIG(wstatus));
        } else if (WIFCONTINUED(wstatus)) {
            printf("continued\n");
        }
    }
    while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

    gen_ctx_destroy(crack_ctx);
    msg_destroy(crack_ctx->msgqid);

    return(EXIT_SUCCESS);
}
