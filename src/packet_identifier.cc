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
    // memcpy(p+1, &id, 4);
    std::copy(&id, &id+4, p+1);
    std::copy(data, data+8, p+5);
    *(p+13) = DELIM;
}

std::string CANPacket::Str()
{
    std::string d = Utils::StrFmt("CANPacket{ id=%b data={ ", id);
    for (int i = 0; i < 8; i++)
        d += Utils::StrFmt("%x ", data[i]);
    return d + "} }";
}

std::vector<CANPacket> PacketIdentifier::IdentifyPackets(uint8_t* buffer, size_t size) 
{
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

void TestPacketIdentifier(PacketIdentifier& identifier) 
{
    std::vector<CANPacket> packets;

    uint8_t a[8] = { 0x00, DELIM, 0x00, 0x00, DELIM, 0x00, DELIM, 0x00};
    packets.push_back(CANPacket(0b0001111111111111111111111111111, a));


    uint8_t b[8] = { 0x00, 0xFF, 0x03, 0xE1, 0x07, 0x93, DELIM, 0xF5};
    packets.push_back(CANPacket(0b0001111111110010011111111000111, b));

    uint8_t buff[32]{0};
    for (int i = 0; i < packets.size(); i++) {
        packets[i].EmplaceFrame(buff + i * 14);
    }
    for (int i = 0; i < sizeof(buff); i++)
        printf("%x ", buff[i]);
    std::cout << "\n";

    std::vector<CANPacket> detected = identifier.IdentifyPackets(buff, sizeof(buff));

    for (CANPacket packet : detected) {
        std::cout << packet.Str() << "\n";
    }

}