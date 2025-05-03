#ifndef _SOCKET_MANAGER_H_
#define _SOCKET_MANAGER_H_

#include "../lib/cpputil/utils.h"
#include <cstdint>
#include <iostream>
#include <set>
#include <thread>
#include <vector>

#define ASIO_STANDALONE
#include "../lib/websocket/websocketpp/config/asio_no_tls.hpp"
#include "../lib/websocket/websocketpp/server.hpp"

#include "packet_mapper.h" // MappedPacket

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;


//
// Maintains the websocket connection and provides the 
// service to stream packets over the network.
// 
// Future reference:
// https://github.com/zaphoyd/websocketpp/blob/master/examples/broadcast_server/broadcast_server.cpp
//
class SocketManager {
    typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

    server server;
    con_list connections;
    std::thread server_thread;

public:
    inline void OnConnOpen(connection_hdl hdl)
    {
        connections.insert(hdl);
    }
    inline void OnConnClose(connection_hdl hdl)
    {
        connections.erase(hdl);
    }
    inline void OnRecvMessage(connection_hdl hdl, server::message_ptr msg)
    {
    }

    inline void Run()
    {
        while (true)
        {
            try {
                server.run();
            } catch (const std::runtime_error& e)
            {
                std::cout << "[SocketManager] caught exception " << e.what() << "\n";
            }
        }
    }
    void TransmitUpdatedFrame(MappedPacket& update);
    void Start(uint16_t port);
    SocketManager()
    {
    }
};

void TestSocketManager(SocketManager& socket);

#endif
