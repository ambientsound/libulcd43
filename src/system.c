#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"
#include "ulcd43.h"

/**
 * Global send and receive buffer
 */
extern char cmdbuf[4096];
extern char recvbuf[2];


int
ulcd_get_display_model(struct ulcd_t *ulcd)
{
    param_t size;
    int s = pack_uints(cmdbuf, 1, GET_DISPLAY_MODEL);
    if (ulcd_send_recv_ack_word(ulcd, cmdbuf, s, &size)) {
        return ulcd->error;
    }
    if (ulcd_recv(ulcd, ulcd->model, size)) {
        return ulcd->error;
    }
    ulcd->model[size] = '\0';
    return ERROK;
}

int
ulcd_get_spe_version(struct ulcd_t *ulcd)
{
    int s = pack_uints(cmdbuf, 1, GET_SPE_VERSION);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, &(ulcd->spe_version));
}

int
ulcd_get_pmmc_version(struct ulcd_t *ulcd)
{
    int s = pack_uints(cmdbuf, 1, GET_PMMC_VERSION);
    return ulcd_send_recv_ack_word(ulcd, cmdbuf, s, &(ulcd->pmmc_version));
}

int
ulcd_get_info(struct ulcd_t *ulcd)
{
    if (ulcd_get_display_model(ulcd)) {
        return ulcd->error;
    }
    if (ulcd_get_spe_version(ulcd)) {
        return ulcd->error;
    }
    if (ulcd_get_pmmc_version(ulcd)) {
        return ulcd->error;
    }
    return ERROK;
}
