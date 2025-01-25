#include <random>
// #include "network_manager.h"
#include "packet_identifier.h"
#include "packet_mapper.h"
#include "serial_interface.h"
#include "../lib/cpputil/utils.h"

static const int RATE = 100;

#define VERBOSE_RECV
#define VERBOSE_STATE

int main(int argc, char** argv)
{
    PacketIdentifier identifier{};
    // TestPacketIdentifier1(identifier);
    // TestPacketIdentifier2(identifier);

    PacketMapper mapper{};
    // TestPacketMapper(mapper);

    SerialInterface serial{"/dev/ttys021", 115200};

    uint8_t buffer[14*32]{0};
    while (true) {
        int read = serial.Load(buffer, sizeof(buffer));
        if (read <= 0) continue;

        std::vector<CANPacket> detected = identifier.IdentifyPackets(buffer, sizeof(buffer));

#ifdef VERBOSE_RECV 
        printf("\nreceived %d bytes:\n", read);
        for (int i = 0; i < read; i++) printf("%02x, ", buffer[i]);
        printf("\n\nparsed into %d frames:\n", detected.size());
        for (CANPacket& packet : detected) {
            std::cout << packet.Str() << "\n";
        }
#endif

        for (const CANPacket& packet : detected)
            mapper.MapPacket(packet);

#ifdef VERBOSE_STATE
        std::cout << "\nstate:\n";
        mapper.PrintState();
#endif
    }


    // }
}


