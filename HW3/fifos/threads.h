#ifndef _THREADS
#define _THREADS
#include "types.h"

#define crBegin static int state=0; switch(state) { case 0:

/*
 * ANSI C provides the __LINE__ macro to return the current source line number
 * We can use the line number as a simple form of programmatic state!
 */
#define crReturn(x) do { state=__LINE__; return x; \
                         case __LINE__:; } while (0)
#define crFinish }
pcb * runqueue;
void init_threads(void);
void runqueue_remove();
#endif

