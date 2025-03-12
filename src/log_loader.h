#ifndef _LOG_LOADER_H_ 
#define _LOG_LOADER_H_ 

#include <iostream>
#include <string>
#include <map>

class LogLoader {
    // all of this shit is unsafe. suck my dick rustf@gs
    FILE* fh;
    char buffer[4096];  // one page of memory
    size_t blk_sz, f_sz;

    int64_t LoadAt(size_t i);
    std::map<std::string, double> ParseBuffer();

public:
    LogLoader(const std::string& path);
    inline ~LogLoader() {  fclose(fh);     }
    std::map<std::string, double> Lookup(int64_t key);
};


#endif