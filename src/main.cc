#include <random>
// #include "network_manager.h"
#include "packet_identifier.h"
#include "packet_mapper.h"
#include "serial_interface.h"
#include "../lib/cpputil/utils.h"

static const int RATE = 100;

int main(int argc, char** argv)
{

    PacketIdentifier identifier{};
    // TestPacketIdentifier(identifier);

    PacketMapper mapper{};
    // TestPacketMapper(mapper);


    SerialInterface serial{"/dev/ttys021", 115200};

    uint8_t buffer[64]{0};
    while (true) {
        int read = serial.Load(buffer, sizeof(buffer));
        if (read < 0)
        for (int i = 0; i < read; i++) 
            printf("%2x ", buffer[i]);
        std::cout << "\n\n";




    }
}


