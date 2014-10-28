/* Simple counter program, to check the functionality of do_yield(). Print
   time in seconds.

   Best viewed with tabs set to 4 spaces. */

#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "th.h"
#include "util.h"
#include "printf.h"

/*
 * This thread runs indefinitely, which means that the
 * scheduler should never run out of processes.
 */
void clock_thread(void)
{
    /*
    printf(6, 48, "Extra credit (attempted):");
#ifdef EC_DEADLOCK
    printf(7, 52, "Deadlock Detection");
#else
    printf(7, 52, "-");
#endif

#ifdef EC_PRIORITIES
    printf(8, 52, "Priority Scheduling");
#else
    printf(8, 52, "-");
#endif
*/

    while (TRUE) {
        int t;

        t = (int) do_gettimeofday() / 15000;
        printf(24, 50, "Time (in seconds) : %d", t);
        print_status();
        do_yield();
    }
}
