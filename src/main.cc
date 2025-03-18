#include <random>
#include <signal.h>

#include "packet_identifier.h"
#include "packet_mapper.h"
#include "serial_interface.h"
#include "socket_manager.h"
#include "log_loader.h"
#include "http_handle.h"
#include "../lib/cpputil/utils.h"

<<<<<<< Updated upstream
//#define VERBOSE_RECV    // send info on recv
// #define VERBOSE_STATE   // send info on state update
=======
#ifdef _WIN32
    #include <cstdlib> // For Windows only, added to use functions like exit() and other stuff
    #include <iostream>
    #include <windows.h> // To use SetConsoleCtrlHandler
#else
    // Nothing else needed when running on mac
#endif

#define NO_SERIAL           // do not use serial 
#define VERBOSE_RECV    // send info on recv
#define VERBOSE_STATE   // send info on state update
>>>>>>> Stashed changes
// #define VERBOSE_WS     // send info about ws send

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
<<<<<<< Updated upstream
int Core::Run(const std::string& serial_port, const std::string& cfg_file, uint16_t http_port, uint16_t ws_port) {
    if (!mapper.LoadMappings(cfg_file)) {};
=======
int Core::Run(const std::string& serial_port, const std::string& cfg_file, uint16_t ws_port) {
    
    if (!mapper.LoadMappings(cfg_file))
    {
        std::cerr << "Failed to load mappings from config file\n"; //printing the error message for testing
    }
>>>>>>> Stashed changes
    std::cout << "Parsed the following packet mappings from config:\n";
    std::cout << mapper.Str() << "\n";

    if (!serial.Connect(serial_port, 115200)) return -1;

    http.StartAsync(http_port);
    socket.Start(ws_port);

    uint8_t buffer[14*32]{0};

    uint64_t t_mount = Utils::PreciseTime<int64_t, Utils::t_us>();
    while (true) {
<<<<<<< Updated upstream
        // super-fast scheduler
        uint64_t t_now = Utils::PreciseTime<int64_t, Utils::t_us>();
        if (t_now - t_mount < 1000) continue;
        t_mount = t_now;



        int read = serial.Read(buffer, sizeof(buffer));
=======

#ifdef NO_SERIAL
        uint8_t test[] {0xf5, 0x00, 0x00, 0x00, 0xff, 0x7d, 0x22, 0x53, 0x6f, 0x7b, 0x89, 0xd1, 0x0c, 0xae};
        size_t read = sizeof(test);
        memcpy(buffer, test, sizeof(test));
#else
        size_t read = serial.Read(buffer, sizeof(buffer)); //figure out why this is not working - make a virtual comport to then test this out in a python script
#endif
>>>>>>> Stashed changes
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

        for (const auto& pair : updated) {
#ifdef VERBOSE_WS
            std::cout << pair.first << ": " << pair.second << "\n";
#endif
            socket.TransmitUpdatedPair(pair);
        }

//         // for (const auto& [key, value] : mapper.values) {
//             // std::cout << key << ": " << value << "\n";
//         //     socket.TransmitUpdatedPair({key, value});
//         // }

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
<<<<<<< Updated upstream
    core.Run(serial_port, "test.cfg", 9000, 9003);

=======
    // for windows you need to run it in a com port
    #ifdef _WIN32
        core.Run("COM1", "test.cfg", 9000);
    #else
        core.Run("/dev/ttys023", "test.cfg", 9000);
    #endif
>>>>>>> Stashed changes
}


