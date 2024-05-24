#ifndef WEBSERVER_TCP_SERVER_H
#define WEBSERVER_TCP_SERVER_H
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <memory>
#include <map>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "../common/socket.h"
#include "../common/connection.h"
#include "../thread/thread_pool.h"
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
        _sock->sock_listen(20);
        printf("start main reactor\n");
        _main_reactor->start();
    }
};
#endif //WEBSERVER_TCP_SERVER_H