/* tasks.c: the list of threads and processes to run (DO NOT CHANGE) */

#include "scheduler.h"
#include "th.h"

struct task_info task[] = {
    /* barrier test threads */
    TH(&barrier1), TH(&barrier2), TH(&barrier3),
    /* And this means the ready queue is never empty */
    PROC(PROC1_ADDR)
    };

enum {
    NUM_TASKS = sizeof task / sizeof(struct task_info)
};
