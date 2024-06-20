#ifndef WEBSERVER_CONNECTION_H
#define WEBSERVER_CONNECTION_H
#include <arpa/inet.h>
#include <memory>
#include <vector>
#include "../config/config.h"
#include "socket.h"
#include "../buffer/ring_buffer.h"

class Connection {
private:
    std::shared_ptr<Socket> _sock;
    // sock fd can't identify a connection unique
    // use id identify a connection unique
    long long _id;

    RingBuffer _in_buff;
    RingBuffer _out_buff;

public:
    explicit Connection(const std::shared_ptr<Socket>& fd) : _sock(fd),
            _in_buff(Config::BUFFER_SIZE), _out_buff(Config::BUFFER_SIZE) {
        static long long idx = 0;
        _id = idx++;
    }

    long long id() const {
        return _id;
    }

    void callback(const std::string& msg) {
        _out_buff.write_msg(msg.c_str(), msg.size());
    }

    int send_http() {
        return _out_buff.send_msg(_sock);
    }

    int recv_http(std::vector<std::string>& msgs) {
        return _in_buff.recv_msg(_sock, "\r\n\r\n", msgs);
    }
};
#endif //WEBSERVER_CONNECTION_H

