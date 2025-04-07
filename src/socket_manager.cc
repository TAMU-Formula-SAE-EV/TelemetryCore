#include "socket_manager.h"

 void SocketManager::TransmitUpdatedFrame(MappedPacket& update) {
    std::ostringstream oss;
    update.Stream(oss);     // this is maybe slow?
    const std::string result = oss.str();

    for (auto it = connections.begin(); it != connections.end(); ++it) {
        server.send(*it, result, websocketpp::frame::opcode::text);
    }
}

void SocketManager::Start(uint16_t port) {
    server.init_asio();
    server.clear_access_channels(websocketpp::log::alevel::all);
    server.clear_error_channels(websocketpp::log::elevel::all);

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

    MappedPacket frame("update", 0, 0);
    for (int i = 1; true; i++) {
        if (i % 1000 == 0) i = 1;
        frame.value = i / 2.5;
        frame.timestamp = Utils::PreciseTime<int64_t, Utils::t_us>();
        socket.TransmitUpdatedFrame(frame);
    }
}