#ifndef _HTTP_HANDLE_H_
#define _HTTP_HANDLE_H_

#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "../lib/httplib/httplib.h"

#include "packet_mapper.h"

// 
// HTTPHandle is the object which encapsulates HTTP communication 
// with the client. The server runs in an alternative thread. 
// It servers the assets directory + various endpoints (/datasreams).
// 
class HTTPHandle {
    httplib::Server server{};
    std::thread* thread;
    uint16_t port{0};
    std::string host;
    std::string datastreams;

public:
    // 
    // Construct the object given the assets directory to serve.
    // 
    inline HTTPHandle(const std::string& assets_dir)
    {
        server.set_base_dir(assets_dir);

        server.Get("/datastreams", [&](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Content-Type", "application/json");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Headers", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            res.set_header("Access-Control-Max-Age", "86400");
            res.set_content(this->datastreams, "text/json");
        });
    }

    // 
    // Start the server on an alternative thread.
    // 
    inline void StartAsync(uint16_t port, const std::string& host)
    {
        this->port = port;  // I dont think there is any need to save these to state
        this->host = host;  // but it doesnt hurt to.
        thread = new std::thread([&]() { server.listen(this->host, this->port); });
    }

    // 
    // On destructor stop the server and clean up the threading
    // resources, follows RAII
    // 
    inline ~HTTPHandle()
    {
        server.stop();
        if (thread != nullptr) {
            thread->join();
            delete thread;
        }
    }

    // 
    // Populates the information to be served over the /datastreams endpoint.
    // Basically metadata about all the types of frames that could be sent
    // over WebSockets. We populate this my looking through the packet mappings
    // and constructing a JSON response.
    // 
    inline void RegisterDatastreams(std::map<uint32_t, std::vector<PacketMapping>>& mappings)
    {
        datastreams = "{\"streams\": [";
        for (const auto& [key, value] : mappings) {
            for (const PacketMapping& mapping : value) {
                datastreams += Utils::StrFmt("{\"name\": \"%s\", \"units\": \"%s\", \"short_name\": \"%s\"},",
                                             mapping.identifier, mapping.unit, mapping.identifier);
            }
        }
        datastreams += Utils::StrFmt("{\"name\": \"%s\", \"units\": \"%s\", \"short_name\": \"%s\"},", "timestamp", "s",
                                     "timestamp");
        datastreams.pop_back();
        datastreams += "]}";
    }
};

// 
// Unit test for the HTTPHandle
// 
void TestHTTPHandle()
{
    HTTPHandle handle{"assets"};
    handle.StartAsync(9000, "0.0.0.0");

    for (int i = 0; true; i++) {
        if (i > 1000) {
            i = 0;
            std::cout << "Hi!\n";
        }
    }
}

#endif