#ifndef _PACKET_MAPPER_H_ 
#define _PACKET_MAPPER_H_ 

#include <cstdint>
#include <vector>
#include <map>
#include <iostream>
#include "packet_identifier.h"
#include "../lib/cpputil/utils.h"

bool is_whitespace(const char ch) {
    return (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r');
}

bool between(const char ch, const char min, const char max) {
    return ch >= min && ch <= max;
}

struct PacketMapping { // encapsulates the concept: "identifier": [start, end]
    size_t start;
    size_t end;
    std::string identifier;

    PacketMapping(size_t start, size_t end, const std::string& identifier)
        : start(start), end(end), identifier(identifier) {}

    std::string Str() {
        return Utils::StrFmt("PacketMapping{ range=%d:%d identifier=%s }",
            start, end, identifier);
    }
};

class PacketMapper {
    using fileiter = std::istreambuf_iterator<char>;
public:
    // treemap is intentional, these are bounded to human 
    // ~comprehensable~ size n
    std::map<uint32_t, std::vector<PacketMapping>> mappings{};
    std::map<std::string, int64_t> values{};

    char IterWS(fileiter& it) { // next and skip whitespace
        char n = *it++; 
        while (is_whitespace(*it)) it++;
        return n;
    }

    // to throw when encountering a character that you dont expect
    inline bool Unexpected(const std::string& expected, char unexpected) {
        std::cout << "[ParsingError] Expected " << expected << ", found " << unexpected << "\n";
        return false;
    }

    // to throw when encountering EOF before you would expect
    inline bool BadEOF() { 
        std::cout << "[ParsingError] Reached EOF before expected\n";
        return false;
    }

    // Parses through the can_id at the beginning of a rule
    // wants hex or binary -- valid forms:
    // - 0xf5ae
    // - 0xE1EC
    // - 0b110011001100
    // *note* whitespace breaks are allowed, usually for 
    // making binary formatting easier to read, ex:
    // - 0b 111 101110 110010
    bool ExpectID(fileiter& it, fileiter end, uint32_t& id) {
        if (*it == '0') {
            it++;
            if (*it == 'x') {
                IterWS(it);
                while (it != end) {
                    if (between(*it, '0', '9')) {
                        id <<= 4;
                        id |= (uint8_t)(*it - '0');
                    }
                    else if (between(*it, 'A', 'F')) {
                        id <<= 4;
                        id |= (uint8_t)((*it - 'A') + 10);
                    }
                    else if (between(*it, 'a', 'f')) {
                        id <<= 4;
                        id |= (uint8_t)((*it - 'a') + 10);
                    } else {
                        // look for {
                        if (*it == '{') return true;
                        return Unexpected("0-9 or a-f or A-F", *it);
                    }
                    IterWS(it);
                }
                return BadEOF(); // ? did correctly read but hit end, 
                             // shouldnt really ever happen but
                             // thats not in the scope of this
                             // functions responsibilites
                // hex
            } else if (*it == 'b') {
                IterWS(it);
                while (it != end) {
                    if (*it == '0') {
                        id <<= 1;
                    } else if (*it == '1') {
                        id <<= 1;
                        id |= 1;
                    } else {
                        if (*it == '{') return true;
                        return Unexpected("0 or 1", *it);
                    }
                    IterWS(it);
                }
                return BadEOF();
            } else {
                return Unexpected("x or b", *it);
            }
        } else {
            return Unexpected("0", *it);
        }
    }

    // Parses through the range #-# where # is a digit in range [1,8] 
    // Specifies the start and end byte for a mapping
    // The result is emplaced in first and last as 0 indexed!
    bool ExpectRange(fileiter& it, fileiter end, uint8_t& first, uint8_t& last) {
        while (it != end) {
            if (!between(*it, '1', '8'))
                return Unexpected("1-8", *it);
            first = *it - '1';
            IterWS(it);
            if (*it != '-')
                return Unexpected("-", *it);
            IterWS(it);
            if (!between(*it, '1', '8'))
                return Unexpected("1-8", *it);
            last = *it - '1';
            IterWS(it);
            return true;
        }
        return BadEOF();
    }

    // Expect an identifier -- rules:
    // - first character: a-z or A-Z
    // - subsequent characters: a-z, A-Z, 0-9, or _ (underscore)
    // - no whitespace breaks
    // 
    bool ExpectIdentifier(fileiter& it, fileiter end, std::string& identifier) {
        if (it == end) return BadEOF();
        if (!between(*it, 'a', 'z') && !between(*it, 'A', 'Z')) return Unexpected("a-z or A-Z", *it);
        identifier += *it++;
        while (it != end) {
            if (between(*it, 'a', 'z') || between(*it, 'A', 'Z') || *it == '_') identifier += *it++;
            else {
                if (is_whitespace(*it)) IterWS(it);
                return true;
            }
        }
        return BadEOF();
    }

    bool ParseMappings(const std::string& path) {
        std::ifstream file(path);
        if (!file) {
            std::cout << "Failed to open config file" << std::endl;
            return false;
        }

        fileiter it = std::istreambuf_iterator<char>(file);
        fileiter end = std::istreambuf_iterator<char>();

        while (it != end) {

            uint32_t can_id;
            if (!ExpectID(it, end, can_id)) return false;

            if (*it != '{') return Unexpected("{", *it);
            else IterWS(it);

            std::vector<PacketMapping> mappings;
            while (it != end) {

                uint8_t first, last;
                if (!ExpectRange(it, end, first, last)) return false;

                if (*it != '=') return Unexpected("=", *it);
                else IterWS(it);

                std::string identifier;
                ExpectIdentifier(it, end, identifier);

                PacketMapping mapping{first, last, identifier};
                mappings.push_back(mapping);

                if (*it == '}') {
                    IterWS(it);
                    break;
                } else if (*it == ',') {
                    IterWS(it);
                    continue;
                } else {
                    return Unexpected(", or }", *it);
                }
            }
            AddMappings(can_id, mappings);
            can_id = 0;
        }
        return true;
    }

    void AddMappings(uint32_t can_id, const std::vector<PacketMapping>& mappings) {
        this->mappings[can_id] = mappings;
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

    std::string Str() {
        std::string d = "[\n";
        for (const auto& [can_id, submappings] : mappings) {
            d += Utils::StrFmt("  Mapping{ can_id=%04x submappings={\n", can_id);
            for (auto submapping : submappings) {
                d += "    " + submapping.Str() + "\n";
            }
            d += "  } }\n";
        }
        d += "]\n";
        return d;
    }

    void __dbg() {
        for (const auto& [key, value] : values) {
            std::cout << key << ": " << value << "\n";
        }
    }

    PacketMapper() {
        // AddMappings(0xf5ae, {PacketMapping(0, 3, "velocity"), PacketMapping(4, 7, "acceleration")});
        if (ParseMappings("test.cfg")) {
            std::cout << "Parsed the following packet mappings from config:\n";
            std::cout << Str();
        }
    }
};

void TestPacketMapper(PacketMapper& mapper) {
    uint8_t a[8] = {0};
    int32_t velo = 75080350; 
    memcpy(a, &velo, 4);
    int32_t accel = -650290;
    memcpy(a+4, &accel, 4);
    CANPacket packet(0xF5AE, a);
    std::cout << "\nTesting parsing of packet: " << packet.Str() << "\n";

    std::cout << "\nValues after parsing: \n";
    mapper.MapPacket(packet);
    mapper.__dbg();


}


#endif