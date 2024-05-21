#ifndef WEBSERVER_FILE_HANDLER_H
#define WEBSERVER_FILE_HANDLER_H
#include <string>
#include <fstream>
#include <sys/stat.h>
#include "../http/http.h"

class FileHandler {
private:
    std::string _root;
public:
    explicit FileHandler(std::string root) : _root(root) {}

    std::string read(const std::string& path) const {
        std::ifstream input_file(path);
        return std::string((std::istreambuf_iterator<char>(input_file)),
                      std::istreambuf_iterator<char>());
    }

    std::string wrapper(const std::string& path) const {
        std::string content = read(_root + path);
        std::string suf = path.substr(path.find_last_of('.') + 1);
        return Http::header_wrapper(Stat::OK, content, suf);
    }

    bool check(const std::string& url) const {
        struct stat info;
        std::string path = _root + url;
        if(stat(path.c_str(), &info) == -1) {
            return false;
        }
        return info.st_mode & S_IFREG;
    }
};


#endif //WEBSERVER_FILE_HANDLER_H
