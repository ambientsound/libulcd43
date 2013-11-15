#ifndef _UTIL_H_
#define _UTIL_H_

#include "caso.h"

/* Utility functions */
inline int pack_uint(char *dest, param_t src);
inline void unpack_uint(param_t *dest, const char *src);
inline int pack_uints(char *buffer, int args, ...);
void print_hex(const char *buffer, int size);

/* Send and receive */
int ulcd_recv_ack(struct ulcd_t *ulcd);
int ulcd_send_recv_ack(struct ulcd_t *ulcd, const char *data, int size);
int ulcd_send_recv_ack_data(struct ulcd_t *ulcd, const char *data, int size, void *buffer, int datasize);

#endif /* #ifndef _UTIL_H_ */
