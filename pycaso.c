#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>
#include <stdarg.h>

/* Benchmarking */
#include <time.h>

/* Types */
typedef unsigned int color_t;
typedef unsigned int param_t;

struct point_t {
    unsigned int x;
    unsigned int y;
};

/* Serial API commands */
#define CLEAR_SCREEN 0xffcd
#define TOUCH_GET 0xff37
#define DRAW_POLYGON 0x0013
#define DRAW_POLYGON_FILLED 0x0014

/* Serial API responses */
#define ACK 0x06
#define NAK 0x15

/* Serial API parameters */
#define TOUCH_GET_STATUS 0
#define TOUCH_GET_X 1
#define TOUCH_GET_Y 2
#define TOUCH_MODE_NOTOUCH 0
#define TOUCH_MODE_PRESS 1
#define TOUCH_MODE_RELEASE 2
#define TOUCH_MODE_MOVING 3

/* Global send and receive buffer */
static char cmdbuf[4096];
static char recvbuf[2];


/**
 * Pack an unsigned int into two bytes, little endian.
 */
static inline int
pack_uint(char *dest, param_t src)
{
    dest[0] = (src & 0xff00) >> 8;
    dest[1] = (src & 0x00ff);
    return 2;
}

/**
 * Unpack an unsigned int from two bytes, little endian.
 */
static inline param_t
unpack_uint(const char *src)
{
    return ((src[0] << 8) & 0xff00) | (src[1] & 0x00ff);
}

/**
 * Pack a variable list of unsigned ints into a char pointer.
 */
static inline int
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
 * Debug function
 */
static void
print_hex(const char *buffer, int size)
{
    int i = 0;
    printf("%d bytes: ", size);
    for (i = 0; i < size; i++) {
        printf("0x%x ", buffer[i]);
    }
    printf("\n");
}

int
ulcd_open_port(void)
{
    int fd;

    fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY);
    if (fd == -1) {
        perror("open_port: Unable to open /dev/ttyAMA0 - ");
    } else {
        fcntl(fd, F_SETFL, 0);
    }

    return fd;
}

void
ulcd_set_port_parameters(int fd)
{
    struct termios options;

    tcgetattr(fd, &options);

    /* 115200 baud */
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

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

    /* No timeout, wait for 1 byte */
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;

    tcsetattr(fd, TCSAFLUSH, &options);
}

int
ulcd_send(int fd, const char *data, int size)
{
    int sent = write(fd, data, size);
    if (sent != size) {
        fputs("ulcd_send() failed\n", stderr);
        return -1;
    }
#ifdef DEBUG_SERIAL
    printf("send: ");
    print_hex(data, size);
#endif
    return 0;
}

int
ulcd_recv_ack(int fd)
{
    unsigned char r;
    ssize_t bytes_read;
    if ((bytes_read = read(fd, &r, 1)) != 1) {
        printf("ulcd_recv_ack() failed: got %d bytes, expected 1\n", bytes_read);
        return -1;
    }
    if (r == ACK) {
        return 0;
    } else if (r == NAK) {
        printf("ulcd_recv_ack() failed: got NAK\n");
    }
    return r;
}

int
ulcd_send_recv_ack(int fd, const char *data, int size)
{
    if (ulcd_send(fd, data, size)) {
        printf("ulcd_send() failed\n");
        return 1;
    }
    if (ulcd_recv_ack(fd)) {
        printf("ulcd_recv_ack() failed\n");
        return 2;
    }
    return 0;
}

int
ulcd_send_recv_ack_data(int fd, const char *data, int size, void *buffer, int datasize)
{
    ssize_t bytes_read;
    int r;
   
    if (r = ulcd_send_recv_ack(fd, data, size)) {
        return r;
    }

    if ((bytes_read = read(fd, buffer, datasize)) != datasize) {
        printf("ulcd_recv_ack() failed: got %d bytes, expected %d\n", bytes_read, datasize);
        return -1;
    }

#ifdef DEBUG_SERIAL
    printf("read: ");
    print_hex(buffer, datasize);
#endif

    return 0;
}

/*********************************************************
 *                   API implementation                  *
 *********************************************************/

int
ulcd_gfx_cls(int fd)
{
    pack_uints(cmdbuf, 1, CLEAR_SCREEN);
    return ulcd_send_recv_ack(fd, cmdbuf, 2);
}

int
ulcd_touch_get(int fd, param_t type, param_t *status)
{
    pack_uints(cmdbuf, 2, TOUCH_GET, 0);
    if (ulcd_send_recv_ack_data(fd, cmdbuf, 4, &recvbuf, 2)) {
        return -1;
    }
    *status = unpack_uint(recvbuf);
    return 0;
}

int
ulcd_gfx_polygon(int fd, color_t color, param_t points, ...)
{
    char *buf;
    va_list lst;
    param_t i;
    struct point_t *p;
    int s;

    s = pack_uints(cmdbuf, 2, DRAW_POLYGON_FILLED, points);
    buf = cmdbuf + s;

    va_start(lst, points);
    for (i = 0; i < points; i++) {
        p = va_arg(lst, struct point_t *);
        s += pack_uint(buf, p->x);
        s += pack_uint(buf+(points*2), p->y);
        buf += 2;
    }
    va_end(lst);
    buf += (points*2);
    s += pack_uint(buf, color);

    print_hex(cmdbuf, s);
    return ulcd_send_recv_ack(fd, cmdbuf, s);
}

int
benchmark(int fd, unsigned long iterations)
{
    unsigned long i;
    struct point_t p1, p2, p3;

    p1.x = 100; p1.y = 100;
    p2.x = 100; p2.y = 200;
    p3.x = 200; p3.y = 150;

    clock_t diff;
    clock_t start = clock();
    param_t st;

    for (i = 0; i < iterations; i++) {
        /*
        p1.x = (p1.x + 1) % 480;
        p2.x = (p2.x + 1) % 480;
        p3.x = (p3.x + 1) % 480;
        if (ulcd_gfx_polygon(fd, 0xffff, 3, &p1, &p2, &p3)) {
            return 1;
        }
        */
        /*
        if (ulcd_gfx_cls(fd)) {
            return 1;
        }
        */
        if (ulcd_touch_get(fd, TOUCH_GET_STATUS, &st)) {
            return -1;
        }
        p1.x = 0;
        p1.y = 0;
        if (st != TOUCH_MODE_NOTOUCH) {
            ulcd_touch_get(fd, TOUCH_GET_X, &p1.x);
            ulcd_touch_get(fd, TOUCH_GET_Y, &p1.y);
        }
        printf("touch_get: %d %d %d\n", st, p1.x, p1.y);
    }

    diff = clock() - start;
    int msec = diff * 1000 / CLOCKS_PER_SEC;

    printf("%d iterations in %.3f seconds\n", iterations, msec/1000.0);
    return 0;
}

int
main(int argc, char** argv)
{
    int fd = ulcd_open_port();

    ulcd_set_port_parameters(fd);

    if (ulcd_gfx_cls(fd)) {
        return 1;
    }
    benchmark(fd, atol(argv[1]));

    close(fd);
}
