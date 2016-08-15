#include <string.h>
#include "test.h"
#include "ipc.h"

int main(int argc, char *argv[])
{
    (void)argc;  // unused
    int qid;

    // preventive cleanup
    qid = msg_qid(argv[0]);
    if (qid != -1)
        msg_destroy(qid);

    qid = msg_create(argv[0]);
    ASSERT_GT(qid, -1);
    ASSERT_EQUAL(qid, msg_qid(argv[0]));

    struct msg msg_s = { 1, "foudil" };
    ASSERT(msg_put(qid, &msg_s));
    struct msg msg_r;
    ASSERT(msg_get(qid, &msg_r, msg_s.type));
    ASSERT_EQUAL(strcmp(msg_s.text, msg_r.text), 0);

    ASSERT(!msg_get(qid, &msg_r, msg_r.type));

    ASSERT(msg_destroy(qid));
    ASSERT_EQUAL(msg_qid(argv[0]), -1);

    return(0);
}