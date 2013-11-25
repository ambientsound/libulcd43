#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "ulcd43.h"
#include "util.h"

extern char cmdbuf[4096];

int
ulcd_move_cursor(struct ulcd_t *ulcd, param_t line, param_t column)
{
    int s = pack_uints(cmdbuf, 3, MOVE_CURSOR, line, column);
    return ulcd_send_recv_ack(ulcd, cmdbuf, s);
}

int
ulcd_txt_putstr(struct ulcd_t *ulcd, const char *str, param_t *slen)
{
    int len = strlen(str);
    int s = pack_uint(cmdbuf, PUT_STR);

    if (len > 511) {
        len = 511;
    }
    strncpy(cmdbuf+s, str, len);
    cmdbuf[s+len] = '\0';

    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s+len+1, slen);
}

int
ulcd_txt_charwidth(struct ulcd_t *ulcd, char c, param_t *width)
{
    int s = pack_uint(cmdbuf, CHAR_WIDTH);
    cmdbuf[s] = c;
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s+1, width);
}

int
ulcd_txt_charheight(struct ulcd_t *ulcd, char c, param_t *height)
{
    int s = pack_uint(cmdbuf, CHAR_HEIGHT);
    cmdbuf[s] = c;
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s+1, height);
}
