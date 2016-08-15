#ifndef IPC_H
#define IPC_H

#include <stdbool.h>
#include <sys/msg.h>

#define MSG_PROJ_ID 'A'
#define MSG_TEXT_LEN 80

struct msg {
    long type;                  // serves as address
    char text[MSG_TEXT_LEN];
};

int msg_create(const char *path);
int msg_qid(const char *path);

bool msg_destroy(const int qid);
bool msg_put(const int qid, const struct msg *msg);
bool msg_get(const int qid, struct msg *msg, long msgtype);

#endif /* IPC_H */
