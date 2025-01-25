#ifndef _SERIAL_STREAMER_
#define _SERIAL_STREAMER_

#include <iostream>

#if defined(WIN32)
    #include "../lib/rs232/rs232.h"
#elif defined(__APPLE__)
    #include "../lib/serialosx/serialosx.h"
#endif

class SerialInterface {

// instance memory needed:
//  on mac: the file descriptor (fd)
//  on win: the com port number (cport_nr)
#if defined(__APPLE__)
    int fd;
#elif defined(WIN32)
    int cport_nr;
#endif

public:
    //"/dev/ttys020"
    SerialInterface(const std::string& port_name, int baudrate);

    size_t Load(uint8_t* buffer, size_t size);

    ~SerialInterface(void);
};

#endif // include gaurd