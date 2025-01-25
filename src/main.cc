#include <random>
#include <signal.h>

#include "packet_identifier.h"
#include "packet_mapper.h"
#include "serial_interface.h"
#include "../lib/cpputil/utils.h"

#define VERBOSE_RECV    // send info on recv
#define VERBOSE_STATE   // send info on state update

// this is the type of function sigaction uses
// typedef for better ergonomics
typedef void (*sig_handler_t)(int);

// collect all the subsystems, organize in convinient
// memory base
class Core {
    // subsystems;
    PacketIdentifier identifier;
    PacketMapper mapper;
    SerialInterface serial;

public:
    Core() : identifier{}, mapper{}, serial{} {}
    int Run(const std::string& serial_port, const std::string& cfg_file);
    sig_handler_t TermHandler(void);
};

// actual main function 
int Core::Run(const std::string& serial_port, const std::string& cfg_file) {

    if (!mapper.LoadMappings(cfg_file)) ;
    std::cout << "Parsed the following packet mappings from config:\n";
    std::cout << mapper.Str() << "\n";

    if (!serial.Connect(serial_port, 115200)) return -1;

    uint8_t buffer[14*32]{0};
    while (true) {
        int read = serial.Read(buffer, sizeof(buffer));
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
}

// function returns a function that is the signal handler
// this inner function has access to all the data inside
// core, but is free of Core::* function pointer 
// namespacing issues
sig_handler_t Core::TermHandler(void) 
{
    return [](int sig) {
        printf("\n\n[Core] process ended on signal = %d\n", sig);
        exit(1); 
    };
}

// OMG ! main function :wow:
int main(int argc, char** argv)
{
    Core core{};

    // Grab the TermHandler and bind it using sigaction
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = core.TermHandler();
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    // Run telemetry with com port and config file
    core.Run("/dev/ttys013", "test.cfg");
}


