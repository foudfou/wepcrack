#ifndef IPC_H
#define IPC_H

#include <stdbool.h>
#include <sys/msg.h>

#define EV_SIGINT   0
#define EV_SIGHUP   1
#define EV_SIGQUIT  2
#define EV_SIGTERM  3
#define EV_SIGUSR1  8
#define EV_SIGALRM  9

#define MSG_PROJ_ID 'A'
#define MSG_TEXT_LEN 80
// man msgsnd(2): « mtype must have a strictly positive integer value »
#define MSG_TYPE_TASK_STATE 0x1
#define MSG_TYPE_WORD_READY 0x2

extern volatile unsigned long events;

struct msg_buf {
    long type;                  // serves as address
    char text[MSG_TEXT_LEN];
};

int msg_create(const char *path);
int msg_qid(const char *path);

bool msg_destroy(const int qid);
bool msg_put(const int qid, const struct msg_buf *msg);
ssize_t msg_get(const int qid, struct msg_buf *msg, const long msgtype);
ssize_t msg_get_sync(const int qid, struct msg_buf *msg, const long msgtype);

int msg_install(const char *path);

void sig_handler(int signo);
bool sig_install();
void sig_children(const pid_t *pids, const int nprocs, const int sig);

#endif /* IPC_H */
