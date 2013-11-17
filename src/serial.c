#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <termios.h>
#include "ulcd43.h"
#include "util.h"

extern char cmdbuf[4096];
extern char recvbuf[2];

/**
 * Baud rates only include types found in Linux. The device also supports other baud rates.
 */
struct baudtable_t baud_index[] = {
    {0, 110},
    {1, 300},
    {2, 600},
    {3, 1200},
    {4, 2400},
    {5, 4800},
    {6, 9600},
    {8, 19200},
    {10, 38400},
    {12, 57600},
    {13, 115200},
    {18, 500000},
    {-1, -1}
};


/**
 * 5.4.1. Set Baud Rate
 *
 * The Set Baud Rate command is used to set the required baud rate. To set the default
 * baud rate, please refer to the instructions in Chapter 2.
 *
 * idx  Req baud    Error   Actual Baud Rate
 * 0    110         0.00%   110
 * 1    300         0.00%   300
 * 2    600         0.01%   600
 * 3    1200        0.03%   1200
 * 4    2400        0.07%   2402
 * 5    4800        0.16%   4808
 * 6    9600        0.33%   9632
 * 7    14400       0.16%   14423
 * 8    19200       0.33%   19264
 * 9    31250       0.00%   31250
 * 10   38400       0.33%   38527
 * 11   56000       0.45%   56250
 * 12   57600       1.73%   58594
 * 13   115200      1.73%   117188
 * 14   128000      4.63%   133929
 * 15   256000      9.86%   281250
 * 16   300000      4.17%   312500
 * 17   375000      7.14%   401786
 * 18   500000      12.50%  562500
 * 19   600000      17.19%  703125
 */
int
ulcd_set_baud_rate(struct ulcd_t *ulcd, param_t baud_rate)
{
    int s;
    struct baudtable_t *t = baud_index;

    while (t->index != -1) {
        if (baud_rate == t->baud_rate) {
            s = pack_uints(cmdbuf, 2, SET_BAUD_RATE, t->index);
            return ulcd_send_recv_ack(ulcd, cmdbuf, s);
        }
        ++t;
    }

    return ulcd_error(ulcd, ERRBAUDRATE, "Baud rate %d is not supported.", baud_rate);
}
