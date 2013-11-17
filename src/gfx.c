#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "ulcd43.h"
#include "util.h"

extern char cmdbuf[4096];
extern char recvbuf[2];

/**
 * 5.2.1
 *
 * The Clear Screen command clears the screen using the current background colour.
 * This command brings some of the settings back to default; such as,
 * * Transparency turned OFF
 * * Outline colour set to BLACK
 * * Opacity set to OPAQUE
 * * Pen set to OUTLINE
 * * Line patterns set to OFF
 * * Right text margin set to full width
 * * Text magnifications set to 1
 * * All origins set to 0:0
 *
 * The alternative to maintain settings and clear screen is to draw a filled
 * rectangle with the required background colour.
 */
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

/**
 * 5.2.31
 *
 * The Contrast Command sets the contrast of the display, or turns it On/Off
 * depending on display model.
 */
int
ulcd_gfx_contrast(struct ulcd_t *ulcd, param_t contrast)
{
    assert(contrast < 16 && contrast >= 0);
    int s = pack_uints(cmdbuf, 2, CONTRAST, contrast);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, NULL);
}

/**
 * Turn the display on.
 *
 * Not part of the official API.
 */
int
ulcd_display_on(struct ulcd_t *ulcd)
{
    return ulcd_gfx_contrast(ulcd, 15);
}

/**
 * Turn the display off.
 *
 * Not part of the official API.
 */
int
ulcd_display_off(struct ulcd_t *ulcd)
{
    return ulcd_gfx_contrast(ulcd, 0);
}
