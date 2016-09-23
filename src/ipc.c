#include <errno.h>
#include <fcntl.h>              /* For O_* constants */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>           /* For S_* constants */
#include <time.h>
#include "bit.h"
#include "ipc.h"

volatile struct sig_events events = { 0 };

static int msg_queue(const char *path, const int mode)
{
    key_t msgkey;
    if ((msgkey = ftok(path, MSG_PROJ_ID)) == -1) {
        perror("ftok");
        return -1;
    }
    int msgqid;
    if ((msgqid = msgget(msgkey, mode)) == -1 && errno != ENOENT)
        perror("msgget");

    return msgqid;
}

int msg_create(const char *path)
{
    return msg_queue(path, IPC_CREAT | 0600);
}

int msg_qid(const char *path)
{
    return msg_queue(path, 0);
}

bool msg_destroy(const int qid)
{
    if (msgctl(qid, IPC_RMID, NULL) == -1) {
        perror ("msgctl");
        return false;
    }
    return true;
}

bool msg_put(const int qid, const struct msg_buf *msg)
{
    if (msgsnd(qid, (void *)msg, sizeof(msg->text), IPC_NOWAIT) == -1) {
        perror("msgsnd");
        return false;
    }
    return true;
}

/* Returns MSG_GET_NOMSG when NOMSG, -1 when ERROR, size of copied text
 * otherwise. */
static ssize_t
msg_get_generic(const int qid, struct msg_buf *msg, const long msgtype,
                const bool async)
{
    int msgflg = MSG_NOERROR;
    if (async)
        msgflg |= IPC_NOWAIT;
    ssize_t size = msgrcv(qid, (void *)msg, sizeof(msg->text), msgtype, msgflg);
    if (size == -1) {
        if (errno == ENOMSG)
            size = MSG_GET_NOMSG;
        else
            perror("msgrcv");
    }
    return size;
}

ssize_t msg_get(const int qid, struct msg_buf *msg, const long msgtype)
{
    return msg_get_generic(qid, msg, msgtype, true);
}

ssize_t msg_get_sync(const int qid, struct msg_buf *msg, const long msgtype)
{
    return msg_get_generic(qid, msg, msgtype, false);
}

int msg_install(const char *path)
{
   int qid = msg_qid(path);
   if (qid != -1) {
       fprintf(stderr, "Found existing message queue. Cleaning...\n");
       struct msg_buf _;
       while (msg_get(qid, &_, 0) > 0);
   }
   return msg_create(path);
}


void sig_handler(int signo, siginfo_t * siginfo, void * uctx)
{
    (void)uctx;  // unused

    events.sigpid = siginfo->si_pid;
    switch (signo) {
    case SIGALRM:
        BIT_SET(events.sigs, EV_SIGALRM);
        break;
    case SIGUSR1:
        BIT_SET(events.sigs, EV_SIGUSR1);
        break;
    case SIGUSR2:
        BIT_SET(events.sigs, EV_SIGUSR2);
        break;
    case SIGTERM: /* ^\ */
    case SIGQUIT: /* ^\ */
    case SIGHUP:
    case SIGINT:  /* ^C */
        BIT_SET(events.sigs, EV_SIGINT);
        break;
    default:
        fprintf(stderr, "caught unhandled %d\n", signo);
    }
}

bool sig_install()
{
    int sig[][2] = {
        {SIGALRM, SA_RESTART|SA_SIGINFO},
        {SIGUSR1, SA_RESTART|SA_SIGINFO},
        {SIGUSR2, SA_RESTART|SA_SIGINFO},
        {SIGINT,  SA_RESTART|SA_SIGINFO},
        {SIGHUP,  SA_RESTART|SA_SIGINFO},
        {SIGQUIT, SA_RESTART|SA_SIGINFO},
        {SIGTERM, SA_RESTART|SA_SIGINFO}
    };
    int len = sizeof(sig) / sizeof(int) / 2;

    struct sigaction sa;
    sa.sa_sigaction = sig_handler;
    for (int i = 0; i < len; i++) {
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = sig[i][1];
        if (sigaction(sig[i][0], &sa, NULL) != 0) {
            perror("sigaction");
            return false;
        }
    }

    return true;
}

void sig_children(const pid_t *pids, const int nprocs, const int sig)
{
    for (int i = 0; i < nprocs; i++)
        kill(pids[i], sig);
}


bool spair_init(struct semphr spair[2]) {
    bool ret = true;
    bool clean = false;
    for (int i = 0; i < 2; ++i) {
        spair[i].semp =
            sem_open(spair[i].name, O_CREAT|O_EXCL, S_IRWXU, spair[i].initval);
        if (spair[i].semp == SEM_FAILED) {
            if (errno == EEXIST)
                clean = true;
            else
                perror(spair[i].name);
            ret = false;
            break;
        }
    }

    if (clean) {
        fprintf(stderr, "Found existing semaphore. Cleaning...\n");
        for (int i = 0; i < 2; ++i) {
            if (sem_unlink(spair[i].name) != 0) {
                perror("sem_unlink");
            }
        }
    }

    return ret;
}

bool spair_end(struct semphr spair[2]) {
    for (int i = 0; i < 2; ++i) {
        if (sem_close(spair[i].semp) != 0) {
            perror(spair[i].name);
            return false;
        }
        if (sem_unlink(spair[i].name) != 0) {
            perror(spair[i].name);
            return false;
        }
    }
    return true;
}
