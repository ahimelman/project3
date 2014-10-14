#include "scheduler.h"

void noop_thread1(void)
{
    while (TRUE) {
        do_yield();
    }
}
