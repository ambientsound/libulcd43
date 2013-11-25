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
ulcd_txt_putch(struct ulcd_t *ulcd, char c)
{
    int s = pack_uints(cmdbuf, 2, PUT_CH, 0x0000 | c);
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

int
ulcd_txt_set_color_fg(struct ulcd_t *ulcd, color_t color, color_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TEXT_FGCOLOUR, color);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_color_bg(struct ulcd_t *ulcd, color_t color, color_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TEXT_BGCOLOUR, color);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_font(struct ulcd_t *ulcd, param_t font, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_FONT_ID, font);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_width(struct ulcd_t *ulcd, param_t multiplier, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_WIDTH, multiplier);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_height(struct ulcd_t *ulcd, param_t multiplier, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_HEIGHT, multiplier);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_xgap(struct ulcd_t *ulcd, param_t pixels, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_X_GAP, pixels);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_ygap(struct ulcd_t *ulcd, param_t pixels, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_Y_GAP, pixels);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_bold(struct ulcd_t *ulcd, param_t value, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_BOLD, value != 0 ? 1 : 0);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_inverse(struct ulcd_t *ulcd, param_t value, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_INVERSE, value != 0 ? 1 : 0);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_italic(struct ulcd_t *ulcd, param_t value, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_ITALIC, value != 0 ? 1 : 0);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_underline(struct ulcd_t *ulcd, param_t value, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_UNDERLINE, value != 0 ? 1 : 0);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_opacity(struct ulcd_t *ulcd, param_t value, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_OPACITY, value != 0 ? 1 : 0);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

int
ulcd_txt_set_attributes(struct ulcd_t *ulcd, param_t value, param_t *prev)
{
    int s = pack_uints(cmdbuf, 2, TXT_ATTRIBUTES, value);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, prev);
}

/**
 * Resets text parameters to sane values.
 *
 * Not part of the official API.
 */
int
ulcd_txt_reset(struct ulcd_t *ulcd)
{
    if (ulcd_txt_set_attributes(ulcd, 0, NULL)) {
        return ulcd->error;
    }
    if (ulcd_txt_set_xgap(ulcd, 0, NULL)) {
        return ulcd->error;
    }
    if (ulcd_txt_set_ygap(ulcd, 0, NULL)) {
        return ulcd->error;
    }
    if (ulcd_txt_set_width(ulcd, 1, NULL)) {
        return ulcd->error;
    }
    if (ulcd_txt_set_height(ulcd, 1, NULL)) {
        return ulcd->error;
    }
    if (ulcd_txt_set_font(ulcd, 0, NULL)) {
        return ulcd->error;
    }
    if (ulcd_txt_set_color_bg(ulcd, 0x0000, NULL)) {
        return ulcd->error;
    }
    if (ulcd_txt_set_color_fg(ulcd, 0xffff, NULL)) {
        return ulcd->error;
    }
    return ulcd->error;
}
