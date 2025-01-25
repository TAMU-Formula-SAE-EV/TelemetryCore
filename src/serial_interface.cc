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
   
#elif defined(WIN32) // function definitions for windows

bool SerialInterface::Connect(const std::string& port_name, int baudrate) 
{
    const char mode[] = {'8','N','1',0};
    const char* target = ("\\\\.\\" + port_name).c_str();
    for (int i = 0; i < sizeof(comports); i++) {
        if (!strcmp(comports[i], target)) {
            if (RS232_OpenComport(i, baudrate, mode, 0)) {
                std::cout << "[SerialError] could not open serial port " << port_name << "\n";
                return false;
            }
            return true;
        }
    }
    return false;
    std::cout << "[SerialError] could not find serial port " << port_name << "\n";
}
size_t SerialInterface::Read(uint8_t* buffer, size_t size)
{
    return RS232_PollComport(cport_nr, buffer, size); 
}
SerialInterface::~SerialInterface(void)
{
    RS232_CloseComport(cport_nr)
}

#endif

