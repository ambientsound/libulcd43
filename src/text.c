#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
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
ulcd_txt_putstr(struct ulcd_t *ulcd, const char *str)
{
    int len = strlen(str);
    int s = pack_uint(cmdbuf, PUT_STR);

    if (len > 511) {
        len = 511;
    }
    strncpy(cmdbuf+s, str, len);
    cmdbuf[s+len] = '\0';

    print_hex(cmdbuf, s+len+1);
    return ulcd_send_recv_ack(ulcd, cmdbuf, s+len+1);
}

/*
    def putStr(self, string):
        if len(string) > 511:
            string = string[:511]
        string += '\0'
        self.send_ack(self.PUT_STR + string)
        return self.recv_word()

    def txt_MoveCursor(self, line, column):
        return self.send_args_ack(self.MOVE_CURSOR, line, column)

*/
