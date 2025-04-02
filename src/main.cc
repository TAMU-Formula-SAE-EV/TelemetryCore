#include <signal.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>

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

#define VERBOSE_RECV 1   // send info on recv
#define VERBOSE_STATE 2  // send info on state update
#define VERBOSE_WS 4     // send info about ws send
#define STATEFULL_WS 8   // collect frames in a map and send all updates at once
#define SPOOF_SERIAL 16  // do not use serial
#define LOG_STATE 32     // log state to file

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
  int Run(const std::string& serial_port, const std::string& cfg_file, uint16_t http_port, uint16_t ws_port,
          uint32_t baud, uint64_t update_epsilion, uint8_t flags);
  sig_handler_t TermHandler(void);
};

// actual main function
int Core::Run(const std::string& serial_port, const std::string& cfg_file, uint16_t http_port, uint16_t ws_port,
              uint32_t baud, uint64_t update_epsilion, uint8_t flags) {
  if (!mapper.LoadMappings(cfg_file)) std::cerr << "[WARNING] Encountered some error while loading mappings from config file\n";

  std::cout << "Parsed the following packet mappings from config:\n";
  std::cout << mapper.Str() << "\n";

  if (flags & SPOOF_SERIAL) {
    std::cout << "[WARNING] Spoofing serial port\n";
  } else {
    if (!serial.Connect(serial_port, baud)) return -1;  // pretty fast baudrate
                                                        // 115200 baud = 14.4 KB/s ~= 14 KB/s
                                                        // (14 KB/s) / (14 B/frame) = 1000 frames/s
  }
  auto& parsed_mappings = mapper.GetMappings();
  http.RegisterDatastreams(parsed_mappings);

  http.StartAsync(http_port);
  socket.Start(ws_port);

  uint8_t buffer[14 * 32]{0};

  uint64_t t_mount = Utils::PreciseTime<int64_t, Utils::t_us>();
  while (true) {
    // super-fast scheduler
    uint64_t t_now = Utils::PreciseTime<int64_t, Utils::t_us>();
    if (t_now - t_mount < update_epsilion) continue;  // use an epsilon oops
    t_mount = t_now;

    size_t read = 0;  
    if (flags & SPOOF_SERIAL) {
      // this code is junk re. ptr handling
      for (auto iter = parsed_mappings.begin(); iter != parsed_mappings.end() && read < sizeof(buffer); iter++) {
        buffer[read++] = 0xf5;
        std::reverse_copy((uint8_t*)&iter->first, (uint8_t*)&iter->first + sizeof(iter->first), buffer + read);
        for (uint8_t i = 0; i < 8; i++) buffer[read + 5 + i] = rand() % 0xFF;  // fucking slow
        read += 13;
        buffer[read++] = 0xae;
      }
    } else {
      read = serial.Read(buffer, sizeof(buffer));
    }
    if (read <= 0) continue;

    // TODO: make this pass by reference
    std::vector<CANPacket> detected = identifier.IdentifyPackets(buffer, sizeof(buffer));

    if (flags & VERBOSE_RECV) {
      printf("\nreceived %d bytes:\n", read);
      for (int i = 0; i < read; i++) printf("%02x, ", buffer[i]);
      printf("\n\nparsed into %d frames:\n", detected.size());
      for (CANPacket& packet : detected) {
        std::cout << packet.Str() << "\n";
      }
    }
    // TODO: move to loop scope and reserve size
    std::vector<std::pair<std::string, double>> updated{};
    for (const CANPacket& packet : detected) {
      mapper.MapPacket(packet, updated);
    }

    if (flags & STATEFULL_WS) {
      for (const auto& pair : updated) {
        if (flags & VERBOSE_WS) {
          std::cout << pair.first << ": " << pair.second << "\n";
        }
        socket.TransmitUpdatedPair(pair);
      }
    } else {
      for (const auto& [key, value] : mapper.values) {
        if (flags & VERBOSE_WS) {
          std::cout << key << ": " << value << "\n";
        }
        socket.TransmitUpdatedPair({key, value});
      }
    }

    if (flags & LOG_STATE) {
      mapper.LogState(log);
    }
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
sig_handler_t Core::TermHandler(void) {
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

#ifdef _WIN32
  #define DEFAULT_SERIAL_PORT "COM3"
#else
  #define DEFAULT_SERIAL_PORT "/dev/ttys1"
#endif

// OMG ! main function :wow:
int main(int argc, char** argv) {
  argparse::ArgumentParser program("telemetrycore", "1.1");

  program.add_argument("-s", "--serial-port")
      //.default_value(DEFAULT_SERIAL_PORT)
      .required()
      .help("The serial port to connect to, or \"spoof\"");

  program.add_argument("-c", "--config-file")
      .required()
      .help("The config file to load mappings from");

  program.add_argument("-p", "--http-port")
      .default_value(9000)
      .help("The port to run the HTTP server on");

  program.add_argument("-w", "--ws-port")
      .default_value(9001)
      .help("The port to run the WebSocket server on");

  program.add_argument("-b", "--baud")
      .default_value(115200)
      .help("The baud rate to connect to the serial port with");

  program.add_argument("-e", "--update-epsilon")
      .default_value(10)
      .help("The update epsilon in microseconds");

  program.add_argument("", "--verbose-all")
      .flag()
      .help("Enable all verbose logging");

  program.add_argument("--verbose-recv")
      .flag()
      .help("Log received data");

  program.add_argument("--verbose-state")
      .flag()
      .help("Log state updates");

  program.add_argument("--verbose-ws") 
      .flag()
      .help("Log WebSocket sends");

  program.add_argument("--log-state")
      .flag()
      .help("Log state to file");

  program.add_argument("--spoof-serial")
      .flag()
      .help("Do not use serial");

  program.add_argument("--stateless-ws")
      .flag()
      .help("Send WebSocket updates as they come");

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(1);
  }

  std::string serial_port = program.get<std::string>("--serial-port");
  std::string cfg_file = program.get<std::string>("--config-file");
  int http_port = program.get<int>("--http-port");
  int ws_port = program.get<int>("--ws-port");
  int baud = program.get<int>("--baud");
  int update_epsilion = program.get<int>("--update-epsilon");
  uint8_t flags = 0;
  if (serial_port == "spoof") flags |= SPOOF_SERIAL;
  if (program["--verbose-all"] == true)   flags |= VERBOSE_RECV | VERBOSE_STATE | VERBOSE_WS;
  if (program["--verbose-recv"] == true) flags |= VERBOSE_RECV;
  if (program["--verbose-state"] == true) flags |= VERBOSE_STATE;
  if (program["--verbose-ws"] == true) flags |= VERBOSE_WS;
  if (program["--log-state"] == true) flags |= LOG_STATE;
  if (program["--spoof-serial"] == true) flags |= SPOOF_SERIAL;
  if (program["--stateless-ws"] == false) flags |= STATEFULL_WS;


  Core core{};

// Grab the TermHandler and bind it using sigaction
#ifdef _WIN32
  // On Windows, use SetConsoleCtrlHandler for handling signals
  SetConsoleCtrlHandler(WindowsCtrlHandler, TRUE);  // did chatgpt generate this? -jus
#else
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = core.TermHandler();
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
#endif

  // Run telemetry with com port and config file
  core.Run(serial_port, cfg_file, http_port, ws_port, baud, update_epsilion, flags);

  return 0;
}
