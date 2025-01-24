#include "packet_identifier.h"

bool CANPacket::operator==(CANPacket& other)
{
    if (&other == this) return true;
    if (other.id != id) return false;
    for (int i = 0; i < 8; i++)
        if (other.data[i] != data[i]) 
            return false;
    return true;
}

void CANPacket::EmplaceFrame(uint8_t* p)
{ 
    *p = DELIM;
    memcpy(p+1, &id, 4);
    // std::copy(&id, &id+4, p+1);
    std::copy(data, data+8, p+5);
    *(p+13) = DELIM;
}

std::string CANPacket::Str()
{
    std::string d = Utils::StrFmt("CANPacket{ id=%s data={ ", 
        std::bitset<32>(id).to_string());
    for (int i = 0; i < 8; i++)
        d += Utils::StrFmt("%x ", data[i]);
    return d + "} }";
}

std::vector<CANPacket> PacketIdentifier::IdentifyPackets(uint8_t* buffer, size_t size) 
{
    std::vector<CANPacket> packets{};
    CANPacket current{};
    uint8_t* last_start; // unused for now -- will use it
    // actual gay shit w/ the *pointer parsing
    for (uint8_t* end = buffer + size, *p = buffer; p < end; ) {
        if (*(p++) == DELIM) {
            for (int i = 0; i <= 24 && p < end; i += 8) {
                current.id |= *(p++) << i;
            }
            for (int i = 0; i < 8 && p < end; i++) {
                current.data[i] = *(p++);
            }
            if (p != end && *(p++) == DELIM) {
                packets.push_back(current);
                current.id = 0;
                memset(current.data, 0, 8);
            } else {
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
                std::cout << "WTF!\n\n";
            }
        } else {
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

void TestPacketIdentifier(PacketIdentifier& identifier) 
{
    std::vector<CANPacket> packets;

    uint8_t a[8] = {0};// 0x00, DELIM, 0x00, 0x00, DELIM, 0x00, DELIM, 0x00};
    memset(a, 0xAA, 8);
    packets.push_back(CANPacket(0b00001111111111111111111111111111, a));

    uint8_t b[8] = {0};// 0x00, 0xFF, 0x03, 0xE1, 0x07, 0x93, DELIM, 0xF5};
    memset(b, 0xBB, 8);
    packets.push_back(CANPacket(0b00001111111111111111111111111111, b));

    uint8_t c[8] = {0};// 0xA1, 0x1C, 0xE0, 0x14, 0xC8, 0xA1, 0x45, 0x0};
    memset(c, 0xCC, 8);
    packets.push_back(CANPacket(0b00001111111111111111111111111111, c));

    uint8_t all[512]{0};
    for (int i = 0; i < packets.size(); i++) {
        packets[i].EmplaceFrame(all + i * 14);
    }

    uint8_t buff1[32]{0};
    memcpy(buff1, all, 32);
    for (int i = 0; i < sizeof(buff1); i++) printf("%2x ", buff1[i]);
    std::cout << "\n";

    uint8_t buff2[32]{0};
    memcpy(buff2, all+32, 32);
    for (int i = 0; i < sizeof(buff2); i++) printf("%2x ", buff2[i]);
    std::cout << "\n";

    std::vector<CANPacket> detected = identifier.IdentifyPackets(buff1, sizeof(buff1));

    for (CANPacket packet : detected) {
        std::cout << packet.Str() << "\n";
    }

}