#ifndef _HTTP_HANDLE_H_ 
#define _HTTP_HANDLE_H_ 

#include <thread>
#include <iostream>
#include <string>
#include <map>

#include "../lib/httplib/httplib.h"

class HTTPHandle {
    httplib::Server server{};
    std::thread* thread;
    uint16_t port{0};

    std::string datastreams;

public:
    inline HTTPHandle() {
        server.set_base_dir("assets");

        server.Get("/datastreams", [&](const httplib::Request& req, httplib::Response& res) {
            res.set_content(this->datastreams, "text/json");
        });
    }

    inline void StartAsync(uint16_t port) 
    {
        this->port = port;
        thread = new std::thread([&]() { server.listen("localhost", this->port); }); 
    }

    inline ~HTTPHandle() {
        server.stop();
        if (thread != nullptr) {
            thread->join();
            delete thread;
        }
    }

    inline void RegisterDatastreams(std::map<uint32_t, std::vector<PacketMapping>> mappings) {
        datastreams = "{\"streams\": [";
        for (const auto& [key, value] : mappings) {
            for (const PacketMapping& mapping : value) {
                datastreams += Utils::StrFmt("{\"name\": \"%s\", \"units\": \"%s\", \"short_name\": \"%s\"},", mapping.identifier, "unit", mapping.identifier);
            }
        }
        datastreams.pop_back();
        datastreams += "]}";
    }
};

void TestHTTPHandle() {
    HTTPHandle handle{};
    handle.StartAsync(9000);

    for (int i = 0; true; i++) {
        if (i > 1000) {
            i = 0;
            std::cout << "Hi!\n";
        }
    }
}


#endif