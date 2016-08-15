#ifndef VARS_H
#define VARS_H

#define EV_SIGINT   (1UL << 0)
#define EV_SIGHUP   (1UL << 1)
#define EV_SIGQUIT  (1UL << 2)
#define EV_SIGTERM  (1UL << 3)
#define EV_SIGUSR1  (1UL << 8)

extern volatile unsigned long int events;

#endif /* VARS_H */
