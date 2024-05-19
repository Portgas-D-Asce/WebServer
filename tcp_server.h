//
// Created by pk on 2024/5/14.
//

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
#include "socket.h"
#include "tcp_connection.h"
#include "thread_pool_copy.h"
#include "main_reactor.h"

template<typename MainMultiplex, typename SubMultiplex>
class TcpServer {
private:
    using MReactor = MainReactor<MainMultiplex, SubMultiplex>;
    std::shared_ptr<Socket> _sock;
    std::shared_ptr<MReactor> _main_reactor;

public:
    explicit TcpServer(int port)
        : _sock(std::make_shared<Socket>()),
        _main_reactor(std::make_shared<MReactor>(_sock)) {
        struct sockaddr_in addr = Socket::sock_address("127.0.0.1", port);
        _sock->sock_bind(addr);
    }

    void start() {
        printf("start listen\n");
        _sock->sock_listen(20);
        printf("start main reactor\n");
        _main_reactor->start();
    }

    //int recv_head(int fd) {
    //    int type = 0, sz = 0;
    //    int cnt = _clients[fd]->sock()->sock_recv((char *)&type, sizeof(type));
    //    if(cnt <= 0) return cnt;
    //    type = ntohl(type);
    //    cnt = _clients[fd]->sock()->sock_recv((char *)&sz, sizeof(sz));
    //    if(cnt <= 0) return cnt;
    //    sz = ntohll(sz);
    //    return 12;
    //}

    //int recv_msg(int fd, long long sz) {
    //    printf("Recv from [%s, %d]: ",
    //           inet_ntoa(_clients[fd]->addr().sin_addr),
    //           ntohs(_clients[fd]->addr().sin_port));
    //    char buf[1024] = {0};
    //    int cnt = _clients[fd]->sock()->sock_recv(buf, sz);
    //    buf[cnt] = '\0';
    //    printf("%d: %s\n", cnt, buf);
    //    return cnt;
    //}

    int listen_fd() const {
        return _sock->fd();
    }
};
#endif //WEBSERVER_TCP_SERVER_H
