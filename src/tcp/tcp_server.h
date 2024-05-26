#ifndef WEBSERVER_TCP_SERVER_H
#define WEBSERVER_TCP_SERVER_H
#include <cstdio>
#include <memory>
#include <arpa/inet.h>
#include "../config/config.h"
#include "../common/socket.h"
#include "../reactor/main_reactor.h"

template<typename MainMultiplex, typename SubMultiplex>
class TcpServer {
private:
    using MReactor = MainReactor<MainMultiplex, SubMultiplex>;
    std::shared_ptr<Socket> _sock;
    std::shared_ptr<MReactor> _main_reactor;

public:
    explicit TcpServer(int port) : _sock(std::make_shared<Socket>()),
        _main_reactor(std::make_shared<MReactor>(_sock)) {

        struct sockaddr_in addr = Socket::sock_address("0.0.0.0", port);
        _sock->sock_bind(addr);
    }

    void start() {
        printf("start listen\n");
        _sock->sock_listen(Config::LISTEN_QUEUE_SIZE);
        printf("start main reactor\n");
        _main_reactor->start();
    }
};
#endif //WEBSERVER_TCP_SERVER_H
