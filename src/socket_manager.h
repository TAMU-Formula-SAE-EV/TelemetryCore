#ifndef _SOCKET_MANAGER_H_ 
#define _SOCKET_MANAGER_H_ 

#include <thread>
#include <cstdint>
#include <vector>
#include <set>
#include <iostream>
#include "../lib/cpputil/utils.h"

#define ASIO_STANDALONE
#include "../lib/websocket/websocketpp/config/asio_no_tls.hpp"
#include "../lib/websocket/websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// For future reference: https://github.com/zaphoyd/websocketpp/blob/master/examples/broadcast_server/broadcast_server.cpp

class SocketManager {
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    server server;
    con_list connections;
    std::thread server_thread;

public:
    inline void OnConnOpen(connection_hdl hdl) { connections.insert(hdl); }
    inline void OnConnClose(connection_hdl hdl) { connections.erase(hdl); }

    inline void OnRecvMessage(connection_hdl hdl, server::message_ptr msg) { }

    void TransmitUpdatedPair(std::pair<std::string, double> update) {
        std::ostringstream oss;
        oss << update.first << ":" << update.second;
        const std::string result = oss.str();

        for (auto it = connections.begin(); it != connections.end(); ++it) {
            server.send(*it, result, websocketpp::frame::opcode::text);
        }
    }

    void Start(uint16_t port) {
        server.listen(port);
        server.start_accept();
        server_thread = std::thread(&SocketManager::Run, this);
    }

    void Run() {
        server.run();
    }

    SocketManager() {
        server.init_asio();

        server.set_open_handler(bind(&SocketManager::OnConnOpen, this,::_1));
        server.set_close_handler(bind(&SocketManager::OnConnClose, this,::_1));
        server.set_message_handler(bind(&SocketManager::OnRecvMessage, this, ::_1, ::_2));
    }
};


void TestSocketManager(SocketManager& socket) 
{
    socket.Start(9002);

    std::pair<std::string, double> pair{"update", 0};
    for (int i = 1; true; i++) {
        if (i % 1000 == 0) i = 1;
        pair.second = i / 2.5;
        std::cout << pair.first << "|" << pair.second << "\n";

        socket.TransmitUpdatedPair(pair);
    }
}


#endif
