#include <signal.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../lib/argparse/include/argparse/argparse.hpp"
#include "../lib/cpputil/utils.h"
#include "http_handle.h"
#include "log_loader.h"
#include "packet_identifier.h"
#include "packet_mapper.h"
#include "serial_interface.h"
#include "socket_manager.h"

#define VERBOSE_RECV 1  // send info on recv
#define VERBOSE_STATE 2 // send info on state update
#define VERBOSE_WS 4    // send info about ws send
#define STATEFULL_WS 8  // collect frames in a map and send all updates at once
#define SPOOF_SERIAL 16 // do not use serial
#define LOG_STATE 32    // log state to file

// this is the type of function sigaction uses
// typedef for better ergonomics
typedef void (*sig_handler_t)(int);

std::string bitsof(uint8_t c)
{
    std::string binary = "";
    for (int i = 7; i >= 0; --i) {
        binary += ((c >> i) & 1) ? '1' : '0';
    }
    return binary;
}

// collect all the subsystems, organize in convinient
// memory base
struct Core {
    // subsystems
    PacketIdentifier identifier;
    PacketMapper mapper;
    SerialInterface serial;
    SocketManager socket;
    HTTPHandle http;
    std::ofstream log;
    uint32_t global_start_time_ms;
    uint64_t global_start_time_us;

    Core(const std::string& assets_dir) : identifier{}, mapper{}, serial{}, socket{}, http{assets_dir}
    {
        log = std::ofstream("db.txt", std::ios::app);
        global_start_time_ms = Utils::PreciseTime<uint32_t, Utils::t_ms>();
        global_start_time_us = Utils::PreciseTime<uint64_t, Utils::t_us>();
    }
    int Run(const std::string& serial_port, const std::string& cfg_file, const std::string& host, uint16_t http_port,
            uint16_t ws_port, uint32_t baud, uint64_t update_epsilion, uint8_t flags);
    sig_handler_t TermHandler(void);
};

// actual main function
int Core::Run(const std::string& serial_port, const std::string& cfg_file, const std::string& host, uint16_t http_port,
              uint16_t ws_port, uint32_t baud, uint64_t update_epsilion, uint8_t flags)
{
    if (!mapper.LoadMappings(cfg_file))
        std::cerr << "[Core:Warning] Encountered some error while loading mappings from config file\n";

    std::cout << "Parsed the following packet mappings from config:\n";
    std::cout << mapper.Str() << "\n";

    if (flags & SPOOF_SERIAL) { std::cout << "[Core:Warning] Spoofing serial port\n"; }
    else {
        if (!serial.Connect(serial_port, baud))
            return -1; // pretty fast baudrate
                       // 115200 baud = 14.4 KB/s ~= 14 KB/s
                       // (14 KB/s) / (14 B/frame) = 1000 frames/s
    }
    auto& parsed_mappings = mapper.GetMappings();
    http.RegisterDatastreams(parsed_mappings);

    http.StartAsync(http_port, host);
    socket.Start(ws_port);

    uint8_t buffer[14 * 256]{0};

    uint64_t t_mount = Utils::PreciseTime<int64_t, Utils::t_us>() - global_start_time_us;
    while (true) {
        // super-fast scheduler
        uint64_t t_now = Utils::PreciseTime<int64_t, Utils::t_us>() - global_start_time_us;
        if (t_now - t_mount < update_epsilion) {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
            continue; // use an epsilon oops
        }
        t_mount = t_now;

        size_t read = 0;
        if (flags & SPOOF_SERIAL) {
            // this code is junk re. ptr handling
            for (auto iter = parsed_mappings.begin(); iter != parsed_mappings.end() && read < sizeof(buffer); iter++) {
                buffer[read++] = 0xf5;
                std::reverse_copy((uint8_t*)&iter->first, (uint8_t*)&iter->first + sizeof(iter->first), buffer + read);
		for (uint8_t* b = buffer + read + 4, i = 0; i < 8; i++)
		{
			b[i] = rand() % 0xFF;
		}

                std::reverse(buffer + read + 4, buffer + read + 4 + 7);
                read += 12;
                buffer[read++] = 0xae;
            }
        }
        else {
            read = serial.Read(buffer, sizeof(buffer));
        }
        if (read <= 0) continue;

        // TODO: make this pass by reference
        std::vector<CANPacket> detected = identifier.IdentifyPackets(buffer, read, global_start_time_ms);

        if (flags & VERBOSE_RECV) {
            printf("\nreceived %d bytes:\n", read);
            for (int i = 0; i < read; i++) 
	    {
		    printf("%02x (", buffer[i]);
		    std::cout << bitsof(buffer[i]) << "), ";
	    }
            printf("\n\nparsed into %d frames:\n", detected.size());
            for (CANPacket& packet : detected) { std::cout << packet.Str() << "\n"; }
        }
        // TODO: move to loop scope and reserve size
        std::vector<MappedPacket> updated{};
        uint32_t fronteir_timestamp = Utils::PreciseTime<uint32_t, Utils::t_ms>() - global_start_time_ms;
        updated.push_back(MappedPacket("timestamp", fronteir_timestamp / 1000.0, fronteir_timestamp));
        for (const CANPacket& packet : detected) { mapper.MapPacket(packet, updated); }

        if (flags & STATEFULL_WS) {
            for (auto& mapped_packet : updated) {
                if (flags & VERBOSE_WS) { 
			mapped_packet.Stream(std::cout); 
			std::cout << "\n";
		}
                socket.TransmitUpdatedFrame(mapped_packet);
            }
        }
        else {
            for (auto& [_, mapped_packet] : mapper.values) {
                if (flags & VERBOSE_WS) { 
			mapped_packet.Stream(std::cout); 
			std::cout << "\n";
		}
                socket.TransmitUpdatedFrame(mapped_packet);
            }
        }

        if (flags & LOG_STATE) { mapper.LogState(log, global_start_time_ms); }
        if (flags & VERBOSE_STATE) {
            std::cout << "\nstate:\n";
            mapper.PrintState();
        }
    }
}

// TODO: im going to revisit all of the below later, im too lazy rn -jus
//       for e im not sure ctrlhdl is exactly what were looking for on
//       win api

// function returns a function that is the signal handler
// this inner function has access to all the data inside
// core, but is free of Core::* function pointer
// namespacing issues
sig_handler_t Core::TermHandler(void)
{
    return [](int sig) {
        printf("\n\n[Core:Fatal] process ended on signal = %d\n", sig);

        std::cout << "\nend state - \n";
        // mapper.PrintState();
        exit(1);
    };
}
// Above code does nto work for windows, so there's a windows only function below
// The POSIX functions like sigaction and sigemptyset are not supported on windows. We are using SetConsoleCtrlHandler
// with with a WindowsCtrlHandler to use Ctrl+C (windows specific button to not get confused).
#ifdef _WIN32
BOOL WINAPI WindowsCtrlHandler(DWORD signal)
{
    if (signal == CTRL_C_EVENT) {
        printf("\n\n[Core:Fatal] process ended on CTRL+C (Windows)\n");
        exit(1);
    }
    return TRUE;
}
#endif

#ifdef _WIN32
#define DEFAULT_SERIAL_PORT "COM3"
#else
#define DEFAULT_SERIAL_PORT "/dev/ttys1"
#endif

template<typename T> // runs O(1) can be slow, but im too fuckin fast boi
T convert_validate_int(const std::string& str, bool& ok, const std::string name, int64_t lower_bound = 0,
                       int64_t upper_bound = 10000000)
{
    std::istringstream iss(str);
    T value;
    iss >> value;
    if (iss.fail() || !iss.eof()) {
        ok = false;
        std::cerr << "[ArgumentError] Input for " << name << " cannot be parsed: " << str << std::endl;
    }
    if (value < lower_bound || value > upper_bound) {
        ok = false;
        std::cerr << "[ArgumentError] Input for " << name << " out of range [" << lower_bound << ", " << upper_bound
                  << "]: " << str << std::endl;
    }
    return value;
}

// OMG ! main function :wow:
int main(int argc, char** argv)
{
    argparse::ArgumentParser program("telemetrycore", "1.1");

    program.add_argument("-s", "--serial-port")
            .required().help("The serial port to connect to, or \"spoof\"");

    program.add_argument("-c", "--config-file")
            .required().help("The config file to load mappings from");

    program.add_argument("-p", "--http-port")
            .default_value("9000").help("The port to run the HTTP server on");

    program.add_argument("-w", "--ws-port")
            .default_value("9001").help("The port to run the WebSocket server on");

    program.add_argument("-b", "--baud")
            .default_value("115200").help("The baud rate to connect to the serial port with");

    program.add_argument("-e", "--update-epsilon")
            .default_value("100").help("The update epsilon in microseconds");

    program.add_argument("-a", "--assets-dir")
            .default_value("assets").help("The directory of the assets to serve");

    program.add_argument("-l", "--localhost")
            .flag().help("Run in localhost mode");

    program.add_argument("--verbose-all")
            .flag().help("Enable all verbose logging");

    program.add_argument("--verbose-recv")
            .flag().help("Log received data");

    program.add_argument("--verbose-state")
            .flag().help("Log state updates");

    program.add_argument("--verbose-ws")
            .flag().help("Log WebSocket sends");

    program.add_argument("--log-state")
            .flag().help("Log state to file");

    program.add_argument("--spoof-serial")
            .flag().help("Do not use serial");

    program.add_argument("--stateless-ws")
            .flag().help("Send WebSocket updates as they come");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        exit(1);
    }

    auto serial_port = program.get<std::string>("--serial-port");
    auto cfg_file = program.get<std::string>("--config-file");
    auto assets_dir = program.get<std::string>("--assets-dir");

    if (std::filesystem::is_directory(assets_dir)) { std::cout << "[Core] assets dir: " << assets_dir << "\n"; }
    else {
        std::cerr << "[ArgumentError] assets dir not found, exiting...\n";
        return -1;
    }

    bool ok = true; // this is aids but its an easy fix shut the fuck up
    auto http_port = convert_validate_int<uint16_t>(program.get<std::string>("--http-port"), ok, "http-port", 0, 65535);
    auto ws_port = convert_validate_int<uint16_t>(program.get<std::string>("--ws-port"), ok, "ws-port", 0, 65535);
    auto baud = convert_validate_int<uint32_t>(program.get<std::string>("--baud"), ok, "baud", 0, 1152000);
    auto update_epsilion = convert_validate_int<int64_t>(program.get<std::string>("--update-epsilon"), ok,
                                                         "update-epsilon", 0, 1000000);
    if (!ok) {
        std::cerr << "[ArgumentError] Invalid arguments, exiting...\n";
        return -1;
    }

    uint8_t flags = 0;
    if (serial_port == "spoof") flags |= SPOOF_SERIAL;
    if (program["--verbose-all"] == true) flags |= VERBOSE_RECV | VERBOSE_STATE | VERBOSE_WS;
    if (program["--verbose-recv"] == true) flags |= VERBOSE_RECV;
    if (program["--verbose-state"] == true) flags |= VERBOSE_STATE;
    if (program["--verbose-ws"] == true) flags |= VERBOSE_WS;
    if (program["--log-state"] == true) flags |= LOG_STATE;
    if (program["--spoof-serial"] == true) flags |= SPOOF_SERIAL;
    if (program["--stateless-ws"] == false) flags |= STATEFULL_WS;

    std::string host = (program["--localhost"] == true) ? "localhost" : "0.0.0.0";
    Core core{assets_dir};

// Grab the TermHandler and bind it using sigaction
#ifdef _WIN32
    // On Windows, use SetConsoleCtrlHandler for handling signals
    SetConsoleCtrlHandler(WindowsCtrlHandler, TRUE); // did chatgpt generate this? -jus
#else
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = core.TermHandler();
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
#endif

    // Run telemetry with com port and config file
    core.Run(serial_port, cfg_file, host, http_port, ws_port, baud, update_epsilion, flags);

    return 0;
}
