#include "network_manager.h"

bool NetworkManager::Start(int port, PacketCallback packet_cb) {
    FD_ZERO(&_fds);

    _net_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_net_socket == -1) {
        Utils::LogFmt("NetworkManager[%s] Failed to create socket. Error: %s", _name, strerror(errno));
        return false;
    }
    _net_addr.sin_family = AF_INET;
    _net_addr.sin_port = htons(port);
    _net_addr.sin_addr.s_addr = INADDR_ANY;
    int option = 1;
    setsockopt(_net_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

    if (bind(_net_socket, (struct sockaddr*)&_net_addr, sizeof(_net_addr)) ==
        -1) {
        Utils::LogFmt("NetworkManager[%s] Socket bind failed. Error: %s", _name, strerror(errno));
        close(_net_socket);
        return false;
    }

    Utils::LogFmt("NetworkManager[%s] Listening on port %i", _name, port);
    listen(_net_socket, 10);
    FD_SET(_net_socket, &_fds);
    _running = true;

    _on_receive = packet_cb;
    _server_thread = std::thread(&NetworkManager::Run, this);
    return true;
}


// Note: https://beej.us/guide/bgnet/html/#select
void NetworkManager::Run() {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    _fdmax = _net_socket;

    struct timeval tv;

    Utils::LogFmt("NetworkManager[%s] Server process running", _name); // todo: fix this log make it better
    while (_running) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        read_fds = _fds;
        if (select(_fdmax + 1, &read_fds, NULL, NULL, &tv) == -1) {
            Utils::ErrFmt("NetworkManager[%s] Error on socket select", _name);
        }

        for (int i = 0; i <= _fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == _net_socket) {
                    AcceptConnection();
                } else {
                    ReceivePacket(i);
                }
            }
        }
    }
}

void NetworkManager::AcceptConnection() {
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof(remoteaddr);
    int conn = accept(_net_socket, (struct sockaddr*)&remoteaddr, &addrlen);
    if (conn == -1) {
        Utils::LogFmt("NetworkManager[%s] Error on packet accept", _name);
    } else {
        FD_SET(conn, &_fds);
        if (conn > _fdmax) {
            _fdmax = conn;
        }
        Utils::LogFmt("NetworkManager[%s] New client %i connected", _name, conn);
        _client_num++;
        _client_sockets.push_back(conn);
    }
}

void NetworkManager::ReceivePacket(int fd) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t nbytes = recv(fd, buffer, sizeof(buffer), 0);
    if (nbytes <= 0) {
        if (nbytes == 0) {
            Utils::LogFmt("NetworkManager[%s] Client %i disconencted", _name, fd);
        } else {
            Utils::LogFmt("NetworkManager[%s] Error on recv", _name);
        }
        _client_sockets.erase(
            std::remove(_client_sockets.begin(), _client_sockets.end(), fd),
            _client_sockets.end());
        _client_num--;
        close(fd);
        FD_CLR(fd, &_fds);
        if (fd == _fdmax) {
            _fdmax--;
        }
    } else  {
        int start = 0;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (buffer[i] == ';') {
                HandlePacket(buffer, start, i - start);
                start = i + 1;
            }
        }
    }
}

void NetworkManager::HandlePacket(char* buffer, int start, size_t len) {
    if (buffer[start] == '\0') return;           // no data available
    if (buffer[start] != '[') return;            // not in valid format
    std::string raw_input(buffer + start, len);  // convert to C++ str
    Utils::LogFmt("NetworkManager[%s] Received valid input: %s\n", _name, raw_input.c_str());

    size_t delim_index = raw_input.find(']');
    std::string cmd = raw_input.substr(1, delim_index - 1);
    std::string data = raw_input.substr(delim_index + 1, std::string::npos);
    Utils::LogFmt("NetworkManager[%s] Parsed frame with header: \"%s\" and content: \"%s\"", cmd.c_str(), data.c_str());

    _on_receive(cmd, data);
}

 void NetworkManager::TransmitFrame(const std::string& header, const std::string& content) {
    TransmitFrame(header.c_str(), header.size(), content.c_str(), content.size());
}

void NetworkManager::TransmitFrame(const char* header_buffer, size_t header_len, const char* content_buffer, size_t content_len) {
    char buffer[BUFFER_SIZE] = {0};
    buffer[0] = '[';
    memcpy(buffer, header_buffer, header_len);
    buffer[header_len] = ']';
    memcpy(buffer + header_len + 1, content_buffer, content_len);
    buffer[header_len + content_len + 1] = ';';

    for (int fd : _client_sockets) {
        Send(fd, buffer, header_len + content_len + 3);
    }
}

void NetworkManager::Send(int fd, const char* buffer, size_t len) {
    size_t total = 0;
    int bytesleft = len;
    int n;

    int count = 0;
    while (total < len) {
        n = send(fd, buffer + total, bytesleft, 0);
        if (n == -1) {
            Utils::LogFmt("NetworkManager[%s] Error on send", _name);
            return;
        }
        total += n;
        bytesleft -= n;
        count++;
        if (count > 100) {
            Utils::LogFmt("NetworkManager[%s] Failed to send", _name);
            break;
        }
    }
}

void NetworkManager::Close() {
    Utils::LogFmt("NetworkManager[%s] Closing connections", _name);
    _running = false;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    for (int fd : _client_sockets) close(fd);
    close(_net_socket);

    _server_thread.join();
}
