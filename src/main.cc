#include <random>
#include "network_manager.h"
#include "../lib/cpputil/utils.h"
#include "packet_identifier.h"

static const int RATE = 100;

// int main(int argc, char** argv)
// {
//     std::random_device rd; 
//     std::mt19937 gen(rd()); 
//     std::uniform_int_distribution<> dist(1000, 9999); 

//     NetworkManager nm("TestServer");
//     nm.Start(8080, [](std::string header, std::string content) {
//         Utils::LogFmt("Main[] Packet received %s %s", header, content);
//     });

//     int i = 0;
//     do {
//         auto start_time = std::chrono::high_resolution_clock::now();
        
//         std::string some_data = Utils::StrFmt("%d %d %d", Utils::TimeMS() & 0xFFFF, (i++), dist(gen));
//         if (i > 100) i = 0;
//         nm.TransmitFrame("update", some_data);

//         auto dt = Utils::ScheduleRate(RATE, start_time);
//         if (dt > 1.0 / RATE) {
//             Utils::LogFmt("Main[] Scheduled update overran by %f s", dt);
//         }
//     } while (true);
//     return 0;
// }

int main() { // testing

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

    std::vector<CANPacket> detected = IdentifyPackets(buff, sizeof(buff));

    for (CANPacket packet : detected) {
        std::cout << packet.Str() << "\n";
    }

}

