//
// Created by pk on 2024/5/20.
//

#ifndef WEBSERVER_FILE_HANDLER_H
#define WEBSERVER_FILE_HANDLER_H
#include <string>
#include <fstream>
#include "../http/http.h"


class FileHandler {
public:
    explicit FileHandler() {}

    std::string read(const std::string& path) {
        std::ifstream input_file(path);
        return std::string((std::istreambuf_iterator<char>(input_file)),
                      std::istreambuf_iterator<char>());
    }

    std::string file_wrapper(const std::string& path) {
        std::string content = read(path);
        std::string suf = path.substr(path.find_last_of('.') + 1);
        return Http::header_wrapper(Stat::OK, content, suf);
    }
};


#endif //WEBSERVER_FILE_HANDLER_H
