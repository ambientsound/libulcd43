#include <stdlib.h>
#include <stdarg.h>
#include "caso.h"
#include "util.h"

extern char cmdbuf[4096];

int
ulcd_move_cursor(struct ulcd_t *ulcd, param_t line, param_t column)
{
    int s = pack_uints(cmdbuf, 3, MOVE_CURSOR, line, column);
    return ulcd_send_recv_ack(ulcd, cmdbuf, s);
}

/*
    def txt_MoveCursor(self, line, column):
        return self.send_args_ack(self.MOVE_CURSOR, line, column)

*/
