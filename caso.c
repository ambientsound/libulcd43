#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>
#include <stdarg.h>

#include "caso.h"

/**
 * Baud rates only include types found in Linux. The device also supports other baud rates.
 */
struct baudtable_t baud_index[] = {
    {0, B110},
    {1, B300},
    {2, B600},
    {3, B1200},
    {4, B2400},
    {5, B4800},
    {6, B9600},
    {8, B19200},
    {10, B38400},
    {12, B57600},
    {13, B115200},
    {18, B500000}
};


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
 * Open the serial port.
 */
int
ulcd_open_serial_port(struct ulcd_t *ulcd)
{
    ulcd->fd = open(ulcd->port, O_RDWR | O_NOCTTY);
    if (ulcd->fd == -1) {
        perror("open_serial_port: Unable to open serial device - ");
        return -1;
    } else {
        fcntl(ulcd->fd, F_SETFL, 0);
    }
    return 0;
}


/**
 * Set serial port parameters to the ones used by uLCD-43.
 */
void
ulcd_set_serial_port_parameters(struct ulcd_t *ulcd)
{
    struct termios options;

    tcgetattr(ulcd->fd, &options);

    /* 115200 baud */
    cfsetispeed(&options, ulcd->baudrate);
    cfsetospeed(&options, ulcd->baudrate);

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

    tcsetattr(ulcd->fd, TCSAFLUSH, &options);
}

int
ulcd_send(struct ulcd_t *ulcd, const char *data, int size)
{
    int sent = write(ulcd->fd, data, size);
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
ulcd_recv_ack(struct ulcd_t *ulcd)
{
    char r;
    ssize_t bytes_read;

    bytes_read = read(ulcd->fd, &r, 1);

#ifdef DEBUG_SERIAL
    printf("read ack: ");
    print_hex(&r, 1);
#endif

    if (bytes_read != 1) {
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
ulcd_send_recv_ack(struct ulcd_t *ulcd, const char *data, int size)
{
    if (ulcd_send(ulcd, data, size)) {
        printf("ulcd_send() failed\n");
        return 1;
    }
    if (ulcd_recv_ack(ulcd)) {
        printf("ulcd_recv_ack() failed\n");
        return 2;
    }
    return 0;
}

int
ulcd_send_recv_ack_data(struct ulcd_t *ulcd, const char *data, int size, void *buffer, int datasize)
{
    ssize_t bytes_read;
    int r;
   
    if ((r = ulcd_send_recv_ack(ulcd, data, size))) {
        return r;
    }

    bytes_read = read(ulcd->fd, buffer, datasize);

#ifdef DEBUG_SERIAL
    printf("read: ");
    print_hex(buffer, datasize);
#endif

    if (bytes_read != datasize) {
        printf("ulcd_recv_ack() failed: got %d bytes, expected %d\n", bytes_read, datasize);
        return -1;
    }

    return 0;
}

int
main(int argc, char** argv)
{
    struct ulcd_t *ulcd;

    ulcd = ulcd_new();
    ulcd->port = "/dev/ttyAMA0";
    ulcd->baudrate = B115200;

    if (ulcd_open_serial_port(ulcd)) {
        return 2;
    }
    ulcd_set_serial_port_parameters(ulcd);

    if (ulcd_gfx_cls(ulcd)) {
        return 1;
    }

    test_touch_draw(ulcd);

    ulcd_free(ulcd);

    return 0;
}
