#include <stdlib.h>
#include <stdarg.h>
#include "caso.h"
#include "util.h"

extern char cmdbuf[4096];
extern char recvbuf[2];

int
ulcd_gfx_cls(struct ulcd_t *ulcd)
{
    int s = pack_uints(cmdbuf, 1, CLEAR_SCREEN);
    return ulcd_send_recv_ack(ulcd, cmdbuf, s);
}

int
ulcd_gfx_filled_circle(struct ulcd_t *ulcd, struct point_t *point, param_t radius, param_t color)
{
    int s = pack_uints(cmdbuf, 5, CIRCLE_FILLED, point->x, point->y, radius, color);
    return ulcd_send_recv_ack(ulcd, cmdbuf, s);
}

int
ulcd_gfx_polygon(struct ulcd_t *ulcd, color_t color, param_t points, ...)
{
    char *buf;
    va_list lst;
    param_t i;
    struct point_t *p;
    int s;

    s = pack_uints(cmdbuf, 2, POLYGON_FILLED, points);
    buf = cmdbuf + s;

    va_start(lst, points);
    for (i = 0; i < points; i++) {
        p = va_arg(lst, struct point_t *);
        s += pack_uint(buf, p->x);
        s += pack_uint(buf+(points*2), p->y);
        buf += 2;
    }
    va_end(lst);
    buf += (points*2);
    s += pack_uint(buf, color);

    print_hex(cmdbuf, s);
    return ulcd_send_recv_ack(ulcd, cmdbuf, s);
}
