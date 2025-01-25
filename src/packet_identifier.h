#ifndef _PACKET_IDENTIFIER_H_ 
#define _PACKET_IDENTIFIER_H_ 

#include <cstdint>
#include <vector>
#include <iostream>
#include "../lib/cpputil/utils.h"

static const uint8_t DELIM_BEGIN = 0xF5;    // A99IE F5AE E1EC
static const uint8_t DELIM_END = 0xAE;

// 1      2     3    4    5    6     7      8       9       10    11     12      13     14
// DELIM | ID | ID | ID | ID | DATA | DATA | DATA | DATA | DATA | DATA | DATA | DATA | DELIM
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
    size_t leftover_size;
    uint8_t leftover[13];

public:
    PacketIdentifier() : leftover_size(0) { memset(leftover, 0, 8); }
    
    // think about optimizations to this return type 
    std::vector<CANPacket> IdentifyPackets(uint8_t* buffer, size_t size);
};

void TestPacketIdentifier1(PacketIdentifier& identifier);
void TestPacketIdentifier2(PacketIdentifier& identifier);

#endif