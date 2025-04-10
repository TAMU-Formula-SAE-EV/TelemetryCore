#include "packet_identifier.h"

//
// Check if two CAN packets are equal, used for validation.
//
bool CANPacket::operator==(CANPacket& other)
{
    if (&other == this) return true;
    if (other.id != id) return false;
    for (int i = 0; i < 8; i++)
        if (other.data[i] != data[i]) return false;
    return true;
}

//
// Given a stream of bytes which represents a packet, construct
// the packet from the bytes between the delims.
//
void CANPacket::EmplaceFrame(uint8_t* p)
{
    *p = DELIM_BEGIN;
    memcpy(p + 1, &id, 4);
    // std::copy(&id, &id+4, p+1);
    std::copy(data, data + 8, p + 5);
    *(p + 13) = DELIM_END;
}

//
// The meaning of this function is left as an excercise for the reader.  
//
std::string CANPacket::Str()
{
    std::string d = Utils::StrFmt("CANPacket{ id=%s data={ ", std::bitset<32>(id).to_string());
    for (int i = 0; i < 8; i++) d += Utils::StrFmt("%02x ", data[i]);
    d += Utils::StrFmt("} timestamp=%d", timestamp);
    return d + " }";
}

// #define HANDLE_LEFTOVER

// 
// Given a buffer of bytes from the serial network + the number of bytes,
// identify the CAN packets and return the ones it finds through a vector. 
//
std::vector<CANPacket> PacketIdentifier::IdentifyPackets(uint8_t* buffer, size_t size, uint32_t global_start_time)
{
    // handle the leftovers from the previous buffer
    if (leftover_size > 0) {
#ifdef HANDLE_LEFTOVER
        int new_size = leftover_size + size;
        uint8_t* bigger_buf = new uint8_t[new_size]{0};
        std::copy(leftover, leftover + leftover_size, bigger_buf);
        std::copy(buffer, buffer + size, bigger_buf + leftover_size);
        // for (int i = 0; i < new_size; i++) printf("%2x ", bigger_buf[i]);
        // std::cout << "\n";
        buffer = bigger_buf;
        size = new_size;
#endif
        leftover_size = 0;
    }

    std::vector<CANPacket> packets{};
    CANPacket current{};
    // uint8_t* start = nullptr;
    // actual gay shit w/ the *pointer parsing
    for (uint8_t *end = buffer + size, *p = buffer; p < end;) {
        uint8_t* start = p;
        if (*(p++) == DELIM_BEGIN) {
            for (int i = 0; i <= 24 && p < end; i += 8) {
                current.id |= *(p++) << (24 - i); // '24-' is used to hot swap endian       @endianswitch
            }
            for (int i = 0; i < 8 && p < end; i++) { current.data[i] = *(p++); }
            // printf("p = %x end = %x\n", p, end);
            // printf("next = %x current = %s\n", *p, current.Str().c_str());
            if (p != end && *(p++) == DELIM_END) {
                current.Timestamp(global_start_time);
                packets.push_back(current);
                current.id = 0;
                memset(current.data, 0, 8);
            }
            else {
                // some error condition, some conditions:
                //
                // we hit were forced through the data,
                // and did not encounter the deliminator
                // (or encoutered the end of the data
                //  and there was no deliminator
                // one thing that can cause this is that
                // a packet was split, so we can save this
                // half of the packet and then try to
                // recover it
                if (p == end) { // we hit the end
                    // how many leftover bytes do we have
                    this->leftover_size = end - start;
                    // and lets copy those over into our leftover buffer
                    std::copy(start, end, this->leftover);
                }
                else {
                    p--;
                    current.id = 0;
                    memset(current.data, 0, 8);
                }
            }
        }
        else {
            // another error condition
            // we are expecting to start a frame
            // but did not
            // we might want to try to parse this
            // second half of a packet, and then
            // join it with the leftovers that we
            // hopefully have of an earlier packet
        }
    }
    return packets;
}

void TestPacketIdentifier1(PacketIdentifier& identifier)
{
    std::vector<CANPacket> packets;

    uint8_t a[8] = {0}; // 0x00, DELIM, 0x00, 0x00, DELIM, 0x00, DELIM, 0x00};
    memset(a, 0xAA, 8);
    packets.push_back(CANPacket(0b00001111111111111111111111111111, a));

    uint8_t b[8] = {0}; // 0x00, 0xFF, 0x03, 0xE1, 0x07, 0x93, DELIM, 0xF5};
    memset(b, 0xBB, 8);
    packets.push_back(CANPacket(0b00001111111111111111111111111111, b));

    uint8_t c[8] = {0}; // 0xA1, 0x1C, 0xE0, 0x14, 0xC8, 0xA1, 0x45, 0x0};
    memset(c, 0xCC, 8);
    packets.push_back(CANPacket(0b00001111111111111111111111111111, c));

    uint8_t all[512]{0};
    for (int i = 0; i < packets.size(); i++) { packets[i].EmplaceFrame(all + i * 14); }

    uint8_t buff1[32]{0};
    memcpy(buff1, all, 32);
    for (int i = 0; i < sizeof(buff1); i++) printf("%2x ", buff1[i]);
    std::cout << "\n";

    uint8_t buff2[32]{0};
    memcpy(buff2, all + 32, 32);
    for (int i = 0; i < sizeof(buff2); i++) printf("%2x ", buff2[i]);
    std::cout << "\n";

    std::vector<CANPacket> detected = identifier.IdentifyPackets(buff1, sizeof(buff1), 0);
    for (CANPacket packet : detected) { std::cout << packet.Str() << "\n"; }

    // to log these make the fields public or smtn
    // std::cout << "leftover_size=" << identifier.leftover_size << " leftover={ ";
    // for (int i = 0; i < sizeof(identifier.leftover); i++) printf("%2x ", identifier.leftover[i]);
    // std::cout << " } \n";

    detected = identifier.IdentifyPackets(buff2, sizeof(buff2), 0);
    for (CANPacket packet : detected) { std::cout << packet.Str() << "\n"; }
}

void TestPacketIdentifier2(PacketIdentifier& identifier)
{
    uint8_t buffer[14 * 4]{0};

    uint8_t partial[]{0xf5, 0x00, 0x00, 0x00, 0xff, 0x9d, 0x21, 0x40, 0x3c, 0xf5, 0xf7, 0xe2, 0xae};
    memcpy(buffer + 7, partial, sizeof(partial));

    uint8_t good[]{0xf5, 0x00, 0x00, 0x00, 0xff, 0x7d, 0x22, 0x53, 0x6f, 0x7b, 0x89, 0xd1, 0x0c, 0xae};
    memcpy(buffer + 7 + sizeof(partial), good, sizeof(good));

    printf("read %d bytes: ", -1);
    for (int i = 0; i < sizeof(buffer); i++) printf("0x%02x, ", buffer[i]);
    std::cout << "\n\n";

    std::vector<CANPacket> detected = identifier.IdentifyPackets(buffer, sizeof(buffer), 0);
    for (CANPacket packet : detected) { std::cout << packet.Str() << "\n"; }
}