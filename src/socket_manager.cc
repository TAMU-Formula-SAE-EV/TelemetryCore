#include "socket_manager.h"

 void SocketManager::TransmitUpdatedFrame(MappedPacket& update) {
    std::ostringstream oss;
    update.Stream(oss);     // this is maybe slow?
    const std::string result = oss.str();

    for (auto it = connections.begin(); it != connections.end(); ++it) {
        ws_server.send(*it, result, websocketpp::frame::opcode::text);
    }
}

void SocketManager::Start(uint16_t port) {
    ws_server.init_asio();
    ws_server.clear_access_channels(websocketpp::log::alevel::all);
    ws_server.clear_error_channels(websocketpp::log::elevel::all);
    ws_server.set_reuse_addr(true);

    ws_server.set_open_handler(bind(&SocketManager::OnConnOpen, this,::_1));
    ws_server.set_close_handler(bind(&SocketManager::OnConnClose, this,::_1));
    ws_server.set_message_handler(bind(&SocketManager::OnRecvMessage, this, ::_1, ::_2));

    ws_server.listen(port);
    ws_server.start_accept();
    server_thread = std::thread(&SocketManager::Run, this);
}

void TestSocketManager(SocketManager& socket)
{
    socket.Start(9002);

    MappedPacket frame("update", 0, 0);
    for (int i = 1; true; i++) {
        if (i % 1000 == 0) i = 1;
        frame.value = i / 2.5;
        frame.timestamp = Utils::PreciseTime<int32_t, Utils::t_ms>();
        socket.TransmitUpdatedFrame(frame);
    }
}