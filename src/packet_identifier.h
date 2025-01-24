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

    bool operator==(CANPacket& other);

    // delim + 12 bytes + delim = 14 bytes
    // mostly for debugging
    void EmplaceFrame(uint8_t* p);

    std::string Str();
}; 

class PacketIdentifier {
public:
    // think about optimizations to this return type 
    std::vector<CANPacket> IdentifyPackets(uint8_t* buffer, size_t size);
};

void TestPacketIdentifier(PacketIdentifier& identifier);

#endif