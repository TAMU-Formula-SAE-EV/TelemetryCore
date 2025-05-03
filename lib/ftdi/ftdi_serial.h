#ifndef __FTDI_SERIAL_H
#define __FTDI_SERIAL_H

#include "ftd2xx.h"
#include <stdexcept>

class FTDISerial {
public:
    FTDISerial() : handle_(nullptr) {}

    bool connect(int baudrate) {
        DWORD num_devs = 0;
        if (FT_CreateDeviceInfoList(&num_devs) != FT_OK || num_devs != 1)
            return false;

        if (FT_Open(0, &handle_) != FT_OK)
            return false;

        if (FT_SetBaudRate(handle_, baudrate) != FT_OK)
            return false;

        FT_SetTimeouts(handle_, 5000, 5000);  // read/write timeout
        return true;
    }

    size_t read(unsigned char* buffer, size_t size) {
        if (!handle_) return 0;

        DWORD bytes_read = 0;
        FT_Read(handle_, buffer, static_cast<DWORD>(size), &bytes_read);
        return static_cast<size_t>(bytes_read);
    }

    void close() {
        if (!handle_) return;
        FT_Close(handle_);
        handle_ = nullptr;
    }

    ~FTDISerial() { close(); }

private:
    FT_HANDLE handle_;
};

#endif // __FTDI_SERIAL_H

// g++ main.cpp -Iinclude -Llib -lftd2xx -o my_program.exe