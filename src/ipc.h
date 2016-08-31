#ifndef IPC_H
#define IPC_H

#include <stdbool.h>
#include <sys/msg.h>

#define MSG_PROJ_ID 'A'
#define MSG_TEXT_LEN 80
#define MSG_TYPE_TASK_STATE 0x0

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

#endif /* IPC_H */
