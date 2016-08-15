#ifndef BIT_H
#define BIT_H

#define BIT_SET(n, x) ((n) |=  (1 << x))
#define BIT_CLR(n, x) ((n) &= ~(1 << x))
#define BIT_TGL(n, x) ((n) ^=  (1 << x))
#define BIT_CHK(n, x) ((n) &   (1 << x))

#endif /* BIT_H */
