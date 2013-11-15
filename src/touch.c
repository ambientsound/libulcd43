#include <stdlib.h>
#include <stdarg.h>
#include "ulcd43.h"
#include "util.h"

extern char cmdbuf[4096];
extern char recvbuf[2];


/**
 * Official API
 */

int
ulcd_touch_set_detect_region(struct ulcd_t *ulcd, struct point_t *p1, struct point_t *p2)
{
    int s = pack_uints(cmdbuf, 5, TOUCH_DETECT_REGION, p1->x, p1->y, p2->x, p2->y);
    return ulcd_send_recv_ack(ulcd, cmdbuf, s);
}

int
ulcd_touch_set(struct ulcd_t *ulcd, param_t type)
{
    int s = pack_uints(cmdbuf, 2, TOUCH_SET, type);
    return ulcd_send_recv_ack(ulcd, cmdbuf, s);
}

int
ulcd_touch_get(struct ulcd_t *ulcd, param_t type, param_t *status)
{
    int s = pack_uints(cmdbuf, 2, TOUCH_GET, type);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, status);
}

/**
 * Initialize the panel's touch mode.
 */
int
ulcd_touch_init(struct ulcd_t *ulcd)
{
    return ulcd_touch_set(ulcd, TOUCH_SET_MODE_INIT);
}

/**
 * Disable the panel's touch mode.
 */
int
ulcd_touch_disable(struct ulcd_t *ulcd)
{
    return ulcd_touch_set(ulcd, TOUCH_SET_MODE_DISABLE);
}

/**
 * Disable the panel's touch mode.
 */
int
ulcd_touch_reset(struct ulcd_t *ulcd)
{
    return ulcd_touch_set(ulcd, TOUCH_SET_MODE_RESET);
}

/**
 * Get a complete touch event with status, x and y coordinates.
 *
 * Not part of the official API.
 */
int
ulcd_touch_get_event(struct ulcd_t *ulcd, struct touch_event_t *ev)
{
    int err;
    if ((err = ulcd_touch_get(ulcd, TOUCH_GET_MODE_STATUS, &(ev->status)))) {
        return err;
    }
    if (ev->status != TOUCH_STATUS_NOTOUCH) {
        if ((err = ulcd_touch_get(ulcd, TOUCH_GET_MODE_GET_X, &(ev->point.x)))) {
            return err;
        }
        if ((err = ulcd_touch_get(ulcd, TOUCH_GET_MODE_GET_Y, &(ev->point.y)))) {
            return err;
        }
    }
    return 0;
}
