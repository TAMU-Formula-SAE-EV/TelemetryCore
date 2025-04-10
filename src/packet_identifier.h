#ifndef _PACKET_IDENTIFIER_H_
#define _PACKET_IDENTIFIER_H_

#include "../lib/cpputil/utils.h"
#include <bitset>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

static const uint8_t DELIM_BEGIN = 0xF5; // A99IE F5AE E1EC
static const uint8_t DELIM_END = 0xAE;

//
// Represents a CAN packet identified pulled from the car.
// The CAN packet is identified from a serial stream like:
// and then the fields of this struct are populated.
// Methods should be self-explanatory.

// 1      2     3    4    5    6     7      8       9       10    11     12      13     14
// DELIM | ID | ID | ID | ID | DATA | DATA | DATA | DATA | DATA | DATA | DATA | DATA | DELIM
//
struct CANPacket {      // 128 bit width
    uint32_t id;        // 32 bit, uses 29 bit
    uint8_t data[8];    // 64 bits
    uint32_t timestamp; // 32 bits

    CANPacket() : id(0), timestamp(0)
    {
        memset(data, 0, 8);
    }

    CANPacket(uint32_t id, uint8_t* data) : id(id)
    {
        std::copy(data, data + 8, this->data);
        Timestamp();
    }

    bool operator==(CANPacket& other); 
    // delim + 12 bytes + delim = 14 bytes
    // mostly for debugging
    void EmplaceFrame(uint8_t* p);

    inline void Timestamp(uint32_t global_start_time = 0)
    {
        timestamp = Utils::PreciseTime<uint32_t, Utils::t_ms>() - global_start_time;
    }

    std::string Str();
};

//
// Identifies packets from continous serial buffers.
//
class PacketIdentifier {
    size_t leftover_size;
    uint8_t leftover[13];

public:
    PacketIdentifier() : leftover_size(0)
    {
        memset(leftover, 0, 8);
    }

    // 
    // Given a buffer of bytes from the serial network + the number of bytes,
    // identify the CAN packets and return the ones it finds through a vector. 
    // TODO: think about optimizations to this return type (switch to pass
    //       by reference vector argument).
    //
    std::vector<CANPacket> IdentifyPackets(uint8_t* buffer, size_t size, uint32_t global_start_time);
};

void TestPacketIdentifier1(PacketIdentifier& identifier);
void TestPacketIdentifier2(PacketIdentifier& identifier);

#endif