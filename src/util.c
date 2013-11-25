#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "config.h"
#include "ulcd43.h"

/**
 * Global send and receive buffer
 */
char cmdbuf[4096];
char recvbuf[2];


/**
 * Pack an unsigned int into two bytes, little endian.
 */
inline int
pack_uint(char *dest, param_t src)
{
    dest[0] = (src & 0xff00) >> 8;
    dest[1] = (src & 0x00ff);
    return 2;
}


/**
 * Unpack an unsigned int from two bytes, little endian.
 */
inline void
unpack_uint(param_t *dest, const char *src)
{
    *dest = ((src[0] << 8) & 0xff00) | (src[1] & 0x00ff);
}


/**
 * Pack a variable list of unsigned ints into a char pointer.
 */
inline int
pack_uints(char *buffer, int args, ...)
{
    int i;
    va_list lst;

    va_start(lst, args);
    for (i = 0; i < args; i++) {
        pack_uint(buffer, va_arg(lst, param_t));
        buffer += 2;
    }
    va_end(lst);

    return args * 2;
}


/**
 * Pack a polygon_t into N bytes.
 */
inline int
pack_polygon(char *dest, struct polygon_t *poly)
{
    int written = 0;
    unsigned int i;
    struct point_t *p;

    assert(poly->num >= 3);

    written += pack_uint(dest, poly->num);
    dest += written;

    for (i = 0; i < poly->num; i++) {
        p = poly->points[i];
        written += pack_uint(dest, p->x);
        written += pack_uint(dest+(poly->num*2), p->y);
        dest += 2;
    }

    return written;
}


/**
 * Create a polygon from points
 */
struct polygon_t *
ulcd_make_polygon(int args, ...)
{
    int i;
    va_list lst;
    struct polygon_t *poly;
    struct point_t *dst, *src;

    poly = malloc(sizeof(struct polygon_t));
    poly->points = malloc(sizeof(struct point_t **));
    poly->num = args;

    dst = poly->points[0];
    va_start(lst, args);
    for (i = 0; i < args; i++) {
        dst = malloc(sizeof(struct point_t));
        src = va_arg(lst, struct point_t *);
        memcpy(dst, src, sizeof(struct point_t));
        poly->points[i] = dst;
    }
    va_end(lst);

    return poly;
}


/**
 * Delete a polygon object
 */
void
ulcd_free_polygon(struct polygon_t *poly)
{
    while (poly->num > 0) {
        free(poly->points[poly->num-1]);
        --(poly->num);
    }
    free(poly->points);
    free(poly);
}

/**
 * Debug function
 */
void
print_hex(const char *buffer, int size)
{
    int i = 0;
    printf("%d bytes: ", size);
    for (i = 0; i < size; i++) {
        printf("0x%x ", buffer[i]);
    }
    printf("\n");
}


/**
 * Create a new struct ulcd_t object.
 */
struct ulcd_t *
ulcd_new(void)
{
    struct ulcd_t *ulcd;
    ulcd = malloc(sizeof(struct ulcd_t));
    memset(ulcd, 0, sizeof(struct ulcd_t));
    ulcd->fd = -1;
    ulcd->baud_rate = 9600;
    ulcd->baud_const = B9600;
    return ulcd;
}


/**
 * Delete a ulcd_t object.
 */
void
ulcd_free(struct ulcd_t *ulcd)
{
    if (ulcd->fd != -1) {
        close(ulcd->fd);
    }
    free(ulcd);
}


/**
 * Set an error message
 */
int
ulcd_error(struct ulcd_t *ulcd, int error, const char *err, ...)
{
    va_list args;

    ulcd->error = error;
    if (err == NULL) {
        ulcd->err[0] = '\0';
    } else {
        va_start(args, err);
        vsnprintf(ulcd->err, STRBUFSIZE, err, args);
        va_end(args);
    }

    return error;
}


/**
 * Open the serial device.
 */
int
ulcd_open_serial_device(struct ulcd_t *ulcd)
{
    ulcd->fd = open(ulcd->device, O_RDWR | O_NOCTTY);
    if (ulcd->fd == -1) {
        return ulcd_error(ulcd, errno, "Unable to open serial device: %s", strerror(errno));
    } else {
        fcntl(ulcd->fd, F_SETFL, 0);
    }
    return ERROK;
}


/**
 * Set serial device parameters to the ones used by uLCD-43.
 */
void
ulcd_set_serial_parameters(struct ulcd_t *ulcd)
{
    struct termios options;

    tcgetattr(ulcd->fd, &options);

    /* Set baud rate */
    cfsetispeed(&options, ulcd->baud_const);
    cfsetospeed(&options, ulcd->baud_const);

    /* 8N1 */
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= CREAD;

    /* Raw input */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* No flow control */
    options.c_iflag |= IGNPAR;
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* Raw output */
    options.c_oflag &= ~OPOST;

    /* 0.5 second timeout */
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 5;

    tcsetattr(ulcd->fd, TCSAFLUSH, &options);
}

int
ulcd_send(struct ulcd_t *ulcd, const char *data, int size)
{
    size_t total = 0;
    size_t sent;
    while (total < size) {
        sent = write(ulcd->fd, data+total, size-total);
        if (sent <= 0) {
            return ulcd_error(ulcd, ERRWRITE, "Unable to send data to device: %s", strerror(errno));
        }
        total += sent;
    }

#ifdef DEBUG_SERIAL
    printf("send: ");
    print_hex(data, size);
#endif

    return ERROK;
}

int
ulcd_recv(struct ulcd_t *ulcd, void *buffer, int size)
{
    size_t bytes_read;
    size_t total = 0;

    while(total < size) {
        bytes_read = read(ulcd->fd, buffer+total, size-total);
        if (bytes_read <= 0) {
            return ulcd_error(ulcd, ERRREAD, "Unable to read data from device: %s", strerror(errno));
        }
        total += bytes_read;
    }

#ifdef DEBUG_SERIAL
    printf("recv: ");
    print_hex(buffer, size);
#endif

    return ERROK;
}


int
ulcd_recv_ack(struct ulcd_t *ulcd)
{
    char r;
    size_t bytes_read;

    if (ulcd_recv(ulcd, &r, 1)) {
        return ulcd->error;
    }

    if (r == ACK) {
        return ERROK;
    } else if (r == NAK) {
        return ulcd_error(ulcd, ERRNAK, "Device sent NAK, expected ACK");
    }

    return ulcd_error(ulcd, ERRUNKNOWN, "Device sent unknown reply `%x' instead of ACK", r);
}

int
ulcd_send_recv_ack(struct ulcd_t *ulcd, const char *data, int size)
{
    if (ulcd_send(ulcd, data, size)) {
        return ulcd->error;
    }
    if (ulcd_recv_ack(ulcd)) {
        return ulcd->error;
    }
    return ERROK;
}

int
ulcd_send_recv_ack_data(struct ulcd_t *ulcd, const char *data, int size, void *buffer, int datasize)
{
    size_t bytes_read;
    size_t total = 0;
   
    if (ulcd_send_recv_ack(ulcd, data, size)) {
        return ulcd->error;
    }

    while(total < datasize) {
        bytes_read = read(ulcd->fd, buffer+total, datasize-total);
        if (bytes_read <= 0) {
            return ulcd_error(ulcd, errno, "Unable to read data from device: %s", strerror(errno));
        }
        total += bytes_read;
    }

#ifdef DEBUG_SERIAL
    printf("recv: ");
    print_hex(buffer, datasize);
#endif

    return ERROK;
}

int
ulcd_send_recv_ack_word(struct ulcd_t *ulcd, const char *data, int size, param_t *param)
{
    char buffer[2];

    if (ulcd_send_recv_ack_data(ulcd, data, size, buffer, 2)) {
        return ulcd->error;
    }

    if (param != NULL) {
        unpack_uint(param, buffer);
    }

    return ERROK;
}


/**
 * Sending wrong commands to the uLCD43 may cause it to lock up. In order to
 * un-fuck it, we send zero-bytes until we get 0x06 0x00 0x09.
 */
int
ulcd_reset(struct ulcd_t *ulcd)
{
    const char target[3] = { 0x06, 0x00, 0x09 };
    char rbuf[STRBUFSIZE];
    int pos = 0;
    int tries;

    for (tries = 0; tries < 10; tries++) {

        if (ulcd_send(ulcd, "\0", 1)) {
            return ulcd->error;
        }

        while (!ulcd_recv(ulcd, rbuf, 1)) {
            if (!memcmp(rbuf, target+pos, 1)) {
                if (++pos == 3) {
                    ulcd_recv(ulcd, rbuf, STRBUFSIZE);
                    return ulcd_error(ulcd, ERROK, "Device has been reset.");
                }
            } else {
                pos = 0;
            }
        }
    }

    return ulcd_error(ulcd, ERRNORESET, "Could not reset device even after %d tries, so gave up.", tries);
}
