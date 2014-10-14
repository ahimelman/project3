/* Just "flies a plane" over the screen. */
#include "common.h"
#include "syslib.h"
#include "util.h"
#include "printf.h"

#define ROWS 4
#define COLUMNS 18

static char picture[ROWS][COLUMNS + 1] = {
    "     ___       _  ",
    " | __\\_\\_o____/_| ",
    " <[___\\_\\_-----<  ",
    " |  o'            "
};

static void draw(int loc_x, int loc_y, int plane);

void _start(void)
{
    int loc_x = 80, loc_y = 1;
    /* Extra Credit *
    int  count = 0;
    unsigned int pid,pri;*/

    while (1) {
        /* erase plane */
        draw(loc_x, loc_y, FALSE);
        loc_x -= 1;
        if (loc_x < -20) {
            loc_x = 80;
        }
        /* draw plane */
        draw(loc_x, loc_y, TRUE);

        /* Extra Credit *
        if (count++ % 100) {
            * print pid and priority *
            pid = getpid();
            pri = getpriority();
            printf(7, 0, "Process %d\tpriority %d", pid, pri);
            if (pri < 64) {
                * increase priority (make plane go faster) *
                setpriority(pri + 1);
            } else {
                * decrease priority (make plane go slower) *
                setpriority(10);
            }
        }*/

        sleep(500);
    }
}

/* draw plane */
static void draw(int loc_x, int loc_y, int plane)
{
    int i, j;

    for (i = 0; i < COLUMNS; i++) {
        for (j = 0; j < ROWS; j++) {
            if (plane == TRUE) {
                print_char(loc_y + j, loc_x + i, picture[j][i]);
            } else {
                print_char(loc_y + j, loc_x + i, ' ');
            }
        }
    }
}
