#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "ipc.h"
#include <string.h>

static int msg_queue(const char *path, const int mode)
{
    key_t msgkey;
    int msgqid;
    if ((msgkey = ftok(path, MSG_PROJ_ID)) == -1) {
        perror("ftok");
        return(-1);
    }
    if ((msgqid = msgget(msgkey, mode)) == -1  && errno != ENOENT)
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
        return(false);
    }
    return(true);
}

bool msg_put(const int qid, const struct msgbuf *msg)
{
    if (msgsnd(qid, (void *)msg, sizeof(msg->text), IPC_NOWAIT) == -1) {
        perror("msgsnd");
        return(false);
    }
    return(true);
}

bool msg_get(const int qid, struct msgbuf *msg, const long msgtype)
{
    if (msgrcv(qid, (void *)msg, sizeof(msg->text), msgtype,
               MSG_NOERROR | IPC_NOWAIT) == -1) {
        if (errno != ENOMSG)
            perror("msgrcv");
        return(false);
    }
    return(true);
}
