#ifndef _PACKET_IDENTIFIER_H_ 
#define _PACKET_IDENTIFIER_H_ 

#include <cstdint>
#include <vector>
#include <iostream>
#include "../lib/cpputil/utils.h"

static const uint8_t DELIM = 0xF5;    // A99I E0F5 AE0E 1EC0

struct CANPacket {      // 96 bit width
    uint32_t id;        // 32 bit, uses 29 bit
    uint8_t data[8];    // 64 bits

    CANPacket() : id(0) { memset(data, 0, 8); }

    CANPacket(uint32_t id, uint8_t* data) : id(id) {
        // memcpy(this->data, data, 8);
        std::copy(data, data+8, this->data);
    }

    bool operator==(CANPacket& other) {
        if (&other == this) return true;
        if (other.id != id) return false;
        for (int i = 0; i < 8; i++)
            if (other.data[i] != data[i]) 
                return false;
        return true;
    }

    // delim + 12 bytes + delim = 14 bytes
    // mostly for debugging
    void EmplaceFrame(uint8_t* p) { 
        *p = DELIM;
        // memcpy(p+1, &id, 4);
        std::copy(&id, &id+4, p+1);
        std::copy(data, data+8, p+5);
        *(p+13) = DELIM;
    }

    std::string Str() {
        std::string d = Utils::StrFmt("CANPacket{ id=%b data={ ", id);
        for (int i = 0; i < 8; i++)
            d += Utils::StrFmt("%x ", data[i]);
        return d + "} }";
    }
}; 

std::vector<CANPacket> IdentifyPackets(uint8_t* buffer, size_t size) {
    std::vector<CANPacket> packets{};
    CANPacket current{};
    // actual gay shit w/ the *pointer parsing
    for (uint8_t* end = buffer + size, *p = buffer; p < end; ) {

        if (*(p++) == DELIM) {
            for (int i = 24; i >= 0; i -= 8) {
                current.id |= *(p++) << i;
            }
            for (int i = 0; i < 8; i++) {
                current.data[i] = *(p++);
            }
            if (*(p++) == DELIM) {
                packets.push_back(current);
            } else {
                // some error condition
            }
        } else {
            // another error condition
        }
    }
    return packets;
}


#endif