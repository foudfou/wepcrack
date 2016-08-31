#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "ipc.h"
#include <string.h>

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

/* Returns -2 when NOMSG, -1 when ERROR, size of copied text otherwise. */
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
            size = -2;
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
