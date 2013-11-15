#include <stdlib.h>
#include <stdarg.h>
#include "ulcd43.h"
#include "util.h"

extern char cmdbuf[4096];
extern char recvbuf[2];

int
ulcd_touch_get(struct ulcd_t *ulcd, param_t type, param_t *status)
{
    pack_uints(cmdbuf, 2, TOUCH_GET, type);
    if (ulcd_send_recv_ack_data(ulcd, cmdbuf, 4, &recvbuf, 2)) {
        return -1;
    }
    unpack_uint(status, recvbuf);
    return 0;
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
