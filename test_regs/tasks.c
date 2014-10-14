/* tasks.c: the list of threads and processes to run (DO NOT CHANGE) */

#include "scheduler.h"
#include "th.h"

struct task_info task[] = {
    TH(&noop_thread1),
    PROC(PROC1_ADDR),
    };

enum {
    NUM_TASKS = sizeof task / sizeof(struct task_info)
};
