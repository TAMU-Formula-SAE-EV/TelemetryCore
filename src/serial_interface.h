#ifndef _SERIAL_STREAMER_
#define _SERIAL_STREAMER_

#include <iostream>

#if defined(_WIN32)
//#include "../lib/rs232/rs232.h"
#include "../lib/ftdi/ftdi_serial.h"
#elif defined(__APPLE__)
#include "../lib/serialosx/serialosx.h"
#endif

#ifdef _WIN32 // added so uint8_t is recognized on windows
#include <cstdint>
#endif
class SerialInterface {

// instance memory needed:
//  on mac: the file descriptor (fd)
//  on win: the com port number (cport_nr)
#if defined(__APPLE__)
    int fd;
#elif defined(_WIN32)
    FTDISerial hftdi{};
#endif

public:
    SerialInterface()
    {
    }

    bool Connect(const std::string& port_name, int baudrate);

    size_t Read(uint8_t* buffer, size_t size);

    ~SerialInterface(void);
};

#endif // include gaurd