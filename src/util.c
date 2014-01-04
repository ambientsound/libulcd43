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
    fprintf(stderr, "%d bytes: ", size);
    for (i = 0; i < size; i++) {
        fprintf(stderr, "0x%x ", buffer[i]);
    }
    fprintf(stderr, "\n");
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
    ulcd->timeout = 500000;
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
    ulcd->fd = open(ulcd->device, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (ulcd->fd == -1) {
        return ulcd_error(ulcd, errno, "Unable to open serial device: %s", strerror(errno));
    }

    ulcd_set_serial_parameters(ulcd);

#ifdef HAVE_SERIAL_BUG
    return ulcd_reset(ulcd);
#endif

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
    options.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE);
    options.c_cflag |= (CS8 | CREAD | CLOCAL);

    /* Raw input */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | ECHONL | ISIG | IEXTEN);

    /* No flow control */
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* Don't munge data */
    options.c_iflag &= ~(ICRNL | IGNCR | INPCK | ISTRIP | ICRNL | IGNBRK);

    /* Raw output */
    options.c_oflag &= ~OPOST;

    /* Timeout handled by select() */
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;

    tcsetattr(ulcd->fd, TCSANOW, &options);
}

static int
ulcd_read_select(struct ulcd_t *ulcd, void *buf, size_t count)
{
    fd_set set;
    struct timeval timeout;
    int retval;

    FD_ZERO(&set);
    FD_SET(ulcd->fd, &set);

    timeout.tv_sec = 0;
    timeout.tv_usec = ulcd->timeout;

    retval = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

    if (retval == 1) {
        return read(ulcd->fd, buf, count);
    } else if (retval == 0) {
        ulcd_error(ulcd, ERRTIMEOUT, "Timed out while reading data from device");
    } else {
        ulcd_error(ulcd, ERRREAD, "Unable to read data from device: %s", strerror(errno));
    }

    return retval;
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

#ifdef SERIAL_DEBUG
    fprintf(stderr, "send: ");
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
        bytes_read = ulcd_read_select(ulcd, buffer+total, size-total);
        if (bytes_read <= 0) {
            return ulcd->error;
        }
        total += bytes_read;
    }

#ifdef SERIAL_DEBUG
    fprintf(stderr, "recv: ");
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
        bytes_read = ulcd_read_select(ulcd, buffer+total, datasize-total);
        if (bytes_read <= 0) {
            return ulcd->error;
        }
        total += bytes_read;
    }

#ifdef SERIAL_DEBUG
    fprintf(stderr, "recv: ");
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
 * Sending wrong commands to the uLCD43 may cause it to lock up or get out of
 * sync. To fix it, we send zero-bytes until we get 0x06 0x00 0x09.
 */
int
ulcd_reset(struct ulcd_t *ulcd)
{
    unsigned long timeout;
    const char target[3] = { 0x06, 0x00, 0x09 };
    char rbuf[STRBUFSIZE];
    int pos = 0;

    timeout = ulcd->timeout;
    ulcd->timeout = 10000;

    while(1) {
        while (!ulcd_recv(ulcd, rbuf, 1)) {
            if (!memcmp(rbuf, target+pos, 1)) {
                if (++pos == 3) {
                    ulcd->timeout = timeout;
                    return ulcd_error(ulcd, ERROK, "Device has been reset.");
                }
            } else {
                pos = 0;
            }
        }

        if (ulcd_send(ulcd, "\0", 1)) {
            ulcd->timeout = timeout;
            return ulcd->error;
        }
    }
}
