#ifndef _NETWORK_MANAGER_H_
#define _NETWORK_MANAGER_H_

#include <cstdio>
#include <cstring>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "../lib/cpputil/utils.h"

// Define packet structure for client->server communication
typedef std::function<void(std::string, std::string)> PacketCallback;


/**
 * NetworkManager -- manages send/receive for custom TCP protocol
 * 
 */
class NetworkManager {
public:
    NetworkManager(std::string name) : _name(name) {};
    ~NetworkManager() { Close(); };

    bool Start(int port, PacketCallback packetCallback);
   
    void Close();

    void TransmitFrame(const std::string& header, const std::string& content);

    void TransmitFrame(const char* header_buffer, size_t header_len, 
        const char* content_buffer, size_t content_len);

private:
    static const int BUFFER_SIZE = 2048;

    void Run();
    void AcceptConnection();
    void ReceivePacket(int fd);
    void HandlePacket(char* buffer, int start, size_t len);

    void Send(int fd, const char* buffer, size_t len);

    std::string _name;

    int _net_socket;
    sockaddr_in _net_addr;

    fd_set _fds;
    int _fdmax{-1};
    int _client_num{0};
    std::vector<int> _client_sockets;

    bool _running{false};
    std::thread _server_thread;

    PacketCallback _on_receive;

};

#endif