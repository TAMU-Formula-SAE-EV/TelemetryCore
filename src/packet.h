#ifndef _PACKET_H_
#define _PACKET_H_

#include <cstdint>
#include <stddef.h>
#include <string>


struct Packet {
    uint8_t* header;
    size_t header_len;

    uint8_t* content;
    size_t content_len;

    uint64_t timestamp;

    Packet() : header(nullptr), header_len(0), content(nullptr), content_len(0), timestamp(0) {};

    Packet(uint64_t timestamp) : header(nullptr), header_len(0), content(nullptr), content_len(0), timestamp(timestamp) {};

    Packet(uint8_t* header, size_t header_len, uint8_t* content, size_t content_len, uint64_t timestamp) 
    : header(header), header_len(header_len), content(content), content_len(content_len), timestamp(timestamp) {};

    Packet(std::string header, std::string content, uint64_t timestamp) : timestamp(timestamp) {
        header_len = header.size();
        content_len = content.size();

        this->header = new uint8_t[header_len];
        this->content = new uint8_t[content_len];

        memcpy(this->header, header.c_str(), header_len);
        memcpy(this->content, content.c_str(), content_len);
    }
};

#endif
