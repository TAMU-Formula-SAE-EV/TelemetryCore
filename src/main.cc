#include <random>
#include <signal.h>

#include "packet_identifier.h"
#include "packet_mapper.h"
#include "serial_interface.h"
#include "socket_manager.h"
#include "log_loader.h"
#include "http_handle.h"
#include "../lib/cpputil/utils.h"

#ifdef _WIN32       // what is this crap?
    #include <cstdlib> // For Windows only, added to use functions like exit() and other stuff
    #include <iostream>
    #include <windows.h> // To use SetConsoleCtrlHandler
#endif

#define VERBOSE_RECV        // send info on recv
#define VERBOSE_STATE       // send info on state update
#define VERBOSE_WS          // send info about ws send
#define STATEFULL_WS        // collect frames in a map and send all updates at once
#define NO_SERIAL           // do not use serial 

// this is the type of function sigaction uses
// typedef for better ergonomics
typedef void (*sig_handler_t)(int);

// collect all the subsystems, organize in convinient
// memory base
struct Core {
    // subsystems;
    PacketIdentifier identifier;
    PacketMapper mapper;
    SerialInterface serial;
    SocketManager socket;
    HTTPHandle http;
    std::ofstream log;

    Core() : identifier{}, mapper{}, serial{}, socket{}, http{} {
        log = std::ofstream("db.txt", std::ios::app);
    }
    int Run(const std::string& serial_port, const std::string& cfg_file, uint16_t http_port, uint16_t ws_port);
    sig_handler_t TermHandler(void);
};

// actual main function 
int Core::Run(const std::string& serial_port, const std::string& cfg_file, uint16_t http_port, uint16_t ws_port) {
    
    // if (!mapper.LoadMappings(cfg_file)) std::cerr << "Failed to load mappings from config file\n"; 

    std::cout << "Parsed the following packet mappings from config:\n";
    std::cout << mapper.Str() << "\n";

    if (!serial.Connect(serial_port, 1152000)) return -1;    // pretty fast baudrate
                                                            // 115200 baud = 14.4 KB/s ~= 14 KB/s
                                                            // (14 KB/s) / (14 B/frame) = 1000 frames/s

    http.RegisterDatastreams(mapper.GetMappings());

    http.StartAsync(http_port);
    socket.Start(ws_port);


    uint8_t buffer[14*32]{0};

    uint64_t t_mount = Utils::PreciseTime<int64_t, Utils::t_us>();
    while (true) {
        // super-fast scheduler
        uint64_t t_now = Utils::PreciseTime<int64_t, Utils::t_us>();
        if (t_now - t_mount < 1000) continue;       // use an epsilon oops
        t_mount = t_now;

#ifdef NO_SERIAL
        // TODO: make a more sophisticated system for this!
        uint8_t test[] {0xf5, 0x00, 0x00, 0x00, 0xff, 0x7d, 0x22, 0x53, 0x6f, 0x7b, 0x89, 0xd1, 0x0c, 0xae};
        size_t read = sizeof(test);
        memcpy(buffer, test, sizeof(test));
#else
        size_t read = serial.Read(buffer, sizeof(buffer)); 
        // WINDOWS: figure out why this is not working - make a virtual comport to then test this out in a python script
#endif
        if (read <= 0) continue;

        // TODO: make this pass by reference 
        std::vector<CANPacket> detected = identifier.IdentifyPackets(buffer, sizeof(buffer));

#ifdef VERBOSE_RECV 
        printf("\nreceived %d bytes:\n", read);
        for (int i = 0; i < read; i++) printf("%02x, ", buffer[i]);
        printf("\n\nparsed into %d frames:\n", detected.size()); 
        for (CANPacket& packet : detected) {
            std::cout << packet.Str() << "\n";
        }
#endif

        // TODO: move to loop scope and reserve size
        std::vector<std::pair<std::string, double>> updated{}; 
        for (const CANPacket& packet : detected) {
            mapper.MapPacket(packet, updated);
        }

#ifndef STATEFULL_WS
        for (const auto& pair : updated) {
#ifdef VERBOSE_WS
            std::cout << pair.first << ": " << pair.second << "\n";
#endif
            socket.TransmitUpdatedPair(pair);
        }
#else
        for (const auto& [key, value] : mapper.values) {
#ifdef VERBOSE_WS
            std::cout << key << ": " << value << "\n";
#endif
            socket.TransmitUpdatedPair({key, value});
        }
#endif

        mapper.LogState(log);
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

        std::cout << "\nend state:\n";
        // mapper.PrintState();
        exit(1); 
    };
}
// Above code does nto work for windows, so there's a windows only function below
// The POSIX functions like sigaction and sigemptyset are not supported on windows. We are using SetConsoleCtrlHandler with with a WindowsCtrlHandler to use Ctrl+C (windows specific button to not get confused).
#ifdef _WIN32
    BOOL WINAPI WindowsCtrlHandler(DWORD signal) {
        if (signal == CTRL_C_EVENT) {
            printf("\n\n[Core] process ended on CTRL+C (Windows)\n");
            exit(1);
        }
        return TRUE;
    }
#endif


// OMG ! main function :wow:
int main(int argc, char** argv)
{
    std::string serial_port = "/dev/ttys1";
    if (argc > 1) { serial_port = argv[1]; }


    Core core{};

    // Grab the TermHandler and bind it using sigaction

    #ifdef _WIN32
        // On Windows, use SetConsoleCtrlHandler for handling signals
        SetConsoleCtrlHandler(WindowsCtrlHandler, TRUE);
    #else
        struct sigaction sigIntHandler;
        sigIntHandler.sa_handler = core.TermHandler();
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);
    #endif

    // Run telemetry with com port and config file
    core.Run(serial_port, "test.cfg", 9000, 9003);
}


