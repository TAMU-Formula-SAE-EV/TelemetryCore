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

    inline void Run() { server.run();}
    void TransmitUpdatedPair(std::pair<std::string, double> update);
    void Start(uint16_t port);
    SocketManager() {  }
};


void TestSocketManager(SocketManager& socket);


#endif
