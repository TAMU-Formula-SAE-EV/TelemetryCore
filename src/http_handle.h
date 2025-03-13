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

public:
    inline HTTPHandle() {
        server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
            res.set_content("Hello World!", "text/plain");
        });
    }

    inline void StartAsync() 
    {
        thread = new std::thread([&]() { server.listen("localhost", 9000); }); 
    }

    inline ~HTTPHandle() {
        server.stop();
        if (thread != nullptr) {
            thread->join();
            delete thread;
        }
    }
};


void TestHTTPHandle() {
    HTTPHandle handle{};
    handle.StartAsync();

    for (int i = 0; true; i++) {
        if (i > 1000) {
            i = 0;
            std::cout << "Hi!\n";
        }
    }
}


#endif