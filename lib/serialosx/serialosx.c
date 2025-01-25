#include "serialosx.h"

int serialosx_open(const char* port_path, int baud_rate) {
    int fd = open(port_path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        fprintf(stderr, "serialosx: error opening serial port %s.\n", port_path);
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, baud_rate);
    cfsetospeed(&options, baud_rate);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_cflag &= ~CRTSCTS;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

int serialosx_read(int fd, unsigned char* buffer, int buffer_size) 
{
    return read(fd, buffer, buffer_size);
    // if (n < 0) fprintf(stderr, "serialosx: error reading from serial port.\n"); 
}

void serialosx_close(int fd) { close(fd); }
