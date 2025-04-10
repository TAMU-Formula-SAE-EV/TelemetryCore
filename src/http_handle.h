#ifndef _HTTP_HANDLE_H_ 
#define _HTTP_HANDLE_H_ 

#include <thread>
#include <iostream>
#include <string>
#include <map>

#include "../lib/httplib/httplib.h"

#include "packet_mapper.h"

class HTTPHandle {
    httplib::Server server{};
    std::thread* thread;
    uint16_t port{0};
    std::string host;
    std::string datastreams;

public:
    inline HTTPHandle(const std::string& assets_dir) {
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

    inline void StartAsync(uint16_t port, const std::string& host) 
    {
        this->port = port;
        this->host = host;
        thread = new std::thread([&]() { server.listen(this->host, this->port); }); 
    }

    inline ~HTTPHandle() {
        server.stop();
        if (thread != nullptr) {
            thread->join();
            delete thread;
        }
    }

    inline void RegisterDatastreams(std::map<uint32_t, std::vector<PacketMapping>>& mappings) {
        datastreams = "{\"streams\": [";
        for (const auto& [key, value] : mappings) {
            for (const PacketMapping& mapping : value) {
                datastreams += Utils::StrFmt("{\"name\": \"%s\", \"units\": \"%s\", \"short_name\": \"%s\"},", mapping.identifier, mapping.unit, mapping.identifier);
            }
        }
        datastreams.pop_back();
        datastreams += "]}";
    }
};

void TestHTTPHandle() {
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