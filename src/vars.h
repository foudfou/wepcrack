#ifndef VARS_H
#define VARS_H

#define EV_SIGINT   0
#define EV_SIGHUP   1
#define EV_SIGQUIT  2
#define EV_SIGTERM  3
#define EV_SIGUSR1  8
#define EV_SIGALRM  9

extern volatile unsigned long events;

#define MSG_TYPE_TASK_STATE 0x10

#endif /* VARS_H */
