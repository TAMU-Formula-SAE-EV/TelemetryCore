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

//old function commented out

// bool SerialInterface::Connect(const std::string& port_name, int baudrate) 
// {
//     const char mode[] = {'8','N','1',0};
//     const char* target = ("\\\\.\\" + port_name).c_str();

//     for (int i = 0; i < sizeof(comports); i++) {
//         if (!strcmp(comports[i], target)) {
//             if (RS232_OpenComport(i, baudrate, mode, 0)) {
//                 std::cout << "[SerialError] could not open serial port " << port_name << "\n";
//                 return false;
//             }
//             return true;
//         }
//     }
//     return false;
//     std::cout << "[SerialError] could not find serial port " << port_name << "\n";
// }

bool SerialInterface::Connect(const std::string& port_name, int baudrate) 
{
   const char mode[] = {'8','N','1',0}; // Serial port mode settings

   // Convert the port name to the format expected by RS232_GetPortnr (so this ensures the port name is in the correct format, which is required by the RS232_GetPortnr function to identify the correct serial port.)

   std::string portStr = port_name;

   // using the library function to get the port number
   this->cport_nr = RS232_GetPortnr(portStr.c_str());

   // seeing if the port number is valid
   if (this->cport_nr == -1) {
       std::cout << "[SerialError] could not find serial port " << port_name << "\n";
       return false;
   }

   // trying to open the serial port
   if (RS232_OpenComport(this->cport_nr, baudrate, mode, 0)) {
       std::cout << "[SerialError] could not open serial port " << port_name << "\n";
       return false;
   }

   return true; // yay I connected to the serial port
}








size_t SerialInterface::Read(uint8_t* buffer, size_t size)
{
    return RS232_PollComport(cport_nr, buffer, size); 
}
SerialInterface::~SerialInterface(void)
{
    RS232_CloseComport(cport_nr);
}

#endif

