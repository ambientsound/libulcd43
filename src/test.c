#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "ulcd43.h"
#include "util.h"

int
test_touch_draw(struct ulcd_t *ulcd)
{
    struct point_t p;
    struct touch_event_t t;

    t.point.x = 0;
    t.point.y = 0;

    while(1) {
        ulcd_touch_get_event(ulcd, &t);
        printf("touch_get: %d %d %d\n", t.status, t.point.x, t.point.y);
        if (p.x != t.point.x && p.y != t.point.y) {
            ulcd_gfx_filled_circle(ulcd, &p, 50, 0x0000);
            if (t.status == TOUCH_STATUS_PRESS || t.status == TOUCH_STATUS_MOVING) {
                ulcd_gfx_filled_circle(ulcd, &(t.point), 50, 0xffff);
            }
            memcpy(&p, &(t.point), sizeof(struct point_t));
        }
        usleep(10000);
    }

    return 0;
}


/*
int
benchmark(struct ulcd_t *ulcd, unsigned long iterations)
{
    unsigned long i;

    clock_t diff;
    clock_t start = clock();
    param_t st;

    for (i = 0; i < iterations; i++) {
    }

    diff = clock() - start;
    int msec = diff * 1000 / CLOCKS_PER_SEC;

    printf("%u iterations in %.3f seconds\n", iterations, (msec/1000.0));
    return 0;
}

*/
