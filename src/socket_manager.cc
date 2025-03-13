#include "socket_manager.h"

 void SocketManager::TransmitUpdatedPair(std::pair<std::string, double> update) {
    std::ostringstream oss;
    oss << update.first << ":" << update.second;
    const std::string result = oss.str();

    for (auto it = connections.begin(); it != connections.end(); ++it) {
        server.send(*it, result, websocketpp::frame::opcode::text);
    }
}

void SocketManager::Start(uint16_t port) {
    server.init_asio();

    server.set_open_handler(bind(&SocketManager::OnConnOpen, this,::_1));
    server.set_close_handler(bind(&SocketManager::OnConnClose, this,::_1));
    server.set_message_handler(bind(&SocketManager::OnRecvMessage, this, ::_1, ::_2));

    server.listen(port);
    server.start_accept();
    server_thread = std::thread(&SocketManager::Run, this);
}


void TestSocketManager(SocketManager& socket)
{
    socket.Start(9002);

    std::pair<std::string, double> pair{"update", 0};
    for (int i = 1; true; i++) {
        if (i % 1000 == 0) i = 1;
        pair.second = i / 2.5;
        // std::cout << pair.first << "|" << pair.second << "\n";

        socket.TransmitUpdatedPair(pair);
    }
}