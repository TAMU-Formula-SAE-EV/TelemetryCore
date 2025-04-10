#include "log_loader.h"

int64_t LogLoader::LoadAt(size_t i)
{
    fseek(fh, i, SEEK_SET);
    fread(buffer, sizeof(buffer), 1, fh);
    char *at = strchr(buffer, '@') + 1, *end;
    return strtol(at, &end, 10);
}

std::map<std::string, double> LogLoader::ParseBuffer()
{
    char* blk_begin = strchr(strchr(buffer, '@'), '\n'); // advance to @ then to \n
    char* blk_end = strchr(blk_begin, '@');              // the next @ is the end
    std::map<std::string, double> entries;
    for (char* k = blk_begin; k < blk_end;) {
        char *eq = strchr(k + 1, '='), *end; // grab the eq
        if (eq > blk_end) break;
        std::string key(k + 1, eq);
        entries[key] = strtod(eq + 1, &k);
    }
    return entries;
}

LogLoader::LogLoader(const std::string& path)
{
    fh = fopen(path.c_str(), "r");
    if (fh == nullptr) { /* handle some issue */
    }
    fread(buffer, sizeof(buffer), 1, fh);
    blk_sz = strchr(buffer + 1, '@') - buffer;
    fseek(fh, 0, SEEK_END);
    f_sz = ftell(fh);
    fseek(fh, 0, SEEK_SET);
}

std::map<std::string, double> LogLoader::Lookup(int64_t key)
{
    size_t left = 0, right = f_sz - 1;
    while (left <= right) {
        size_t midpt = left + (right - left) / 2;
        std::cout << "Searching @ " << midpt << "\n";
        int64_t at = LoadAt(midpt);
        if (at == key) { return ParseBuffer(); }
        else if (at < key) {
            left = midpt + 1;
        }
        else {
            right = midpt - 1;
        }
    }
}
