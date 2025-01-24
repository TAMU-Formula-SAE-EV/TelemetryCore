#ifndef _PACKET_MAPPER_H_ 
#define _PACKET_MAPPER_H_ 

#include <cstdint>
#include <vector>
#include <map>
#include <iostream>
#include "packet_identifier.h"
#include "../lib/cpputil/utils.h"

struct PacketMapping { // encapsulates the concept: "identifier": [start, end]
    size_t start;
    size_t end;
    std::string identifier;

    PacketMapping(size_t start, size_t end, const std::string& identifier)
        : start(start), end(end), identifier(identifier) {}
};

class PacketMapper {
public:
    // treemap is intentional, these are bounded to human 
    // ~comprehensable~ size n
    std::map<uint32_t, std::vector<PacketMapping>> mappings{};
    std::map<std::string, int64_t> values{};

    void AddMappings(uint32_t identifier, const std::vector<PacketMapping>& mappings) {
        this->mappings[identifier] = mappings;
    }

    void MapPacket(const CANPacket& packet) {
        auto it = mappings.find(packet.id);
        if (it == mappings.end()) return;
        for (const PacketMapping& mapping : it->second) {
            int64_t extracted = 0;
            int width = mapping.end - mapping.start;
            for (int i = mapping.start, j = 0; i <= mapping.end; i++) {
                extracted |= packet.data[i] << (8 * (j++)); // some endian configuration 
                // extracted |= packet.data[i] << (8 * (width - (j++))); // other endian configuration
            }
            values[mapping.identifier] = extracted;
        }
    }

    void __dbg() {
        for (const auto& [key, value] : values) {
            std::cout << key << ": " << value << "\n";
        }
    }

    PacketMapper() {
        AddMappings(0xf5ae, {PacketMapping(0, 3, "velocity"), PacketMapping(4, 7, "acceleration")});
    }
};

void TestPacketMapper(PacketMapper& mapper) {
    uint8_t a[8] = {0};
    int32_t velo = 75080350; 
    memcpy(a, &velo, 4);
    int32_t accel = -650290;
    memcpy(a+4, &accel, 4);
    CANPacket packet(0xF5AE, a);
    std::cout << packet.Str() << "\n";

    mapper.MapPacket(packet);
    mapper.__dbg();
}


#endif