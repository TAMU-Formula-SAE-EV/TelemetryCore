#include "serial_interface.h"

#if defined(__APPLE__) // function definitions for apple

bool SerialInterface::Connect(const std::string& port_name, int baudrate)
{
    if ((this->fd = serialosx_open(port_name.c_str(), baudrate)) == -1) {
        std::cout << "[SerialError] could not open serial port " << port_name << "\n";
        return false;
    }
    return true;
}
size_t SerialInterface::Read(uint8_t* buffer, size_t size)
{
    return serialosx_read(fd, buffer, size);
}
SerialInterface::~SerialInterface(void)
{
    serialosx_close(fd);
}

#elif defined(_WIN32) // function definitions for windows

bool SerialInterface::Connect(const std::string& port_name, int baudrate)
{
    return hftdi.connect(baudrate);
}
size_t SerialInterface::Read(uint8_t* buffer, size_t size)
{
    return hftdi.read(buffer, size);
}
SerialInterface::~SerialInterface(void)
{
    hftdi.close();
}

#endif
