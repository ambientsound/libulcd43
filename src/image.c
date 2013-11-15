#include <stdlib.h>
#include <stdarg.h>
#include "ulcd43.h"
#include "util.h"

extern char cmdbuf[4096];
extern char recvbuf[2];

int
ulcd_image_bitblt(struct ulcd_t *ulcd, struct point_t *point, param_t width, param_t height, const char *buffer)
{
    int s = pack_uints(cmdbuf, 5, BLIT_COM_TO_DISPLAY, point->x, point->y, width, height);
    if (ulcd_send(ulcd, cmdbuf, s)) {
        return -1;
    }
    return ulcd_send_recv_ack(ulcd, buffer, width*height*2);
}
