#include "serialosx.h"


int serialosx_open(const char* port_path, int baud_rate) {
    int fd = open(port_path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        fprintf(stderr, "serialosx: error opening serial port %s.\n", port_path);
        return -1;
    }

    // Configure the serial port
    struct termios options;
    tcgetattr(fd, &options);

    // Set baud rate
    cfsetispeed(&options, baud_rate);
    cfsetospeed(&options, baud_rate);

    // Set 8 data bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // Set no flow control 
    options.c_cflag &= ~CRTSCTS;

    // Set raw input and output
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    // Apply the settings
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

int serialosx_read(int fd, unsigned char* buffer, int buffer_size) 
{
    int bytesRead = read(fd, buffer, buffer_size);
    // if (bytesRead == -1) { 
    //     fprintf(stderr, "serialosx: error reading from serial port.\n"); 
    // }
    return bytesRead;
}

void serialosx_close(int fd) 
{
    close(fd);
}
