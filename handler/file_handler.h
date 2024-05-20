//
// Created by pk on 2024/5/20.
//

#ifndef WEBSERVER_FILE_HANDLER_H
#define WEBSERVER_FILE_HANDLER_H
#include <string>
#include "../common/socket.h"

class FileHandler {
private:
    std::shared_ptr<Socket> _sock;
public:
    explicit FileHandler(const std::shared_ptr<Socket>& sock)
        : _sock(sock) {}

    void operator()(const std::string& url) {

    }
};


#endif //WEBSERVER_FILE_HANDLER_H
