#ifdef __APPLE__

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

int serialosx_open(const char* port_path, int baud_rate);
int serialosx_read(int fd, unsigned char* buffer, int buffer_size);
void serialosx_close(int fd);

#ifdef __cplusplus
}
#endif
