#include <random>
#include "network_manager.h"
#include "packet_identifier.h"
#include "packet_mapper.h"
#include "../lib/cpputil/utils.h"

static const int RATE = 100;

int main(int argc, char** argv)
{
    // PacketIdentifier identifier{};
    // TestPacketIdentifier(identifier);

    PacketMapper mapper{};
    TestPacketMapper(mapper);



    // std::random_device rd; 
    // std::mt19937 gen(rd()); 
    // std::uniform_int_distribution<> dist(1000, 9999); 

    // NetworkManager nm("TestServer");
    // nm.Start(8080, [](std::string header, std::string content) {
    //     Utils::LogFmt("Main[] Packet received %s %s", header, content);
    // });

    // int i = 0;
    // do {
    //     auto start_time = std::chrono::high_resolution_clock::now();
        
    //     std::string some_data = Utils::StrFmt("%d %d %d", Utils::TimeMS() & 0xFFFF, (i++), dist(gen));
    //     if (i > 100) i = 0;
    //     nm.TransmitFrame("update", some_data);

    //     auto dt = Utils::ScheduleRate(RATE, start_time);
    //     if (dt > 1.0 / RATE) {
    //         Utils::LogFmt("Main[] Scheduled update overran by %f s", dt);
    //     }
    // } while (true);
    // return 0;
}


