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

template<typename Multiplex>
class TcpServer {
private:
    std::shared_ptr<Socket> _sock;
    std::map<int, std::shared_ptr<TcpConnection<Multiplex>>> _clients;
    std::shared_ptr<Multiplex> _multiplex;
public:
    explicit TcpServer(int port) {
        _sock = std::make_shared<Socket>();
        _multiplex = std::make_shared<Multiplex>(*this);
        struct sockaddr_in addr = Socket::sock_address("127.0.0.1", port);
        _sock->sock_bind(addr);
    }

    void start() {
        _sock->sock_listen(20);
        _multiplex->dispatch();
    }

    int connect() {
        struct sockaddr_in client{};
        auto fd = _sock->sock_accept(client);
        if(!fd) {
            perror("add client socket");
            return -1;
        }
        _clients[fd->fd()] = std::make_shared<TcpConnection<Multiplex>>(fd, client, _multiplex);
        _clients[fd->fd()]->sock()->sock_nonblock();
        printf("Welcome home %s : %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        return fd->fd();
    }

    void disconnect(int fd) {
        printf("Goodbye %s : %d\n",
               inet_ntoa(_clients[fd]->addr().sin_addr),
               ntohs(_clients[fd]->addr().sin_port));

        _clients.erase(fd);
    }

    int recv_head(int fd) {
        int type = 0, sz = 0;
        int cnt = _clients[fd]->sock()->sock_recv((char *)&type, sizeof(type));
        if(cnt <= 0) return cnt;
        type = ntohl(type);
        cnt = _clients[fd]->sock()->sock_recv((char *)&sz, sizeof(sz));
        if(cnt <= 0) return cnt;
        sz = ntohll(sz);
        return 12;
    }

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

    int recv_msg(int fd) {
        int n = _clients[fd]->recv();
        printf("%d recv len(%d) message\n", fd, n);
        if(n == 0) {
            disconnect(fd);
            return 0;
        }
        if(n == -1) {
            printf("recv error: %d\n", fd);
            return -1;
        }
        return n;
    }

    int listen_fd() const {
        return _sock->fd();
    }
};
#endif //WEBSERVER_TCP_SERVER_H
