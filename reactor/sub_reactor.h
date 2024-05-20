//
// Created by pk on 2024/5/19.
//

#ifndef WEBSERVER_SUB_REACTOR_H
#define WEBSERVER_SUB_REACTOR_H
#include <memory>
#include <map>
#include <functional>
#include "../common/socket.h"
#include "../common/connection.h"

template<typename Multiplex>
class SubReactor {
private:
    std::map<int, std::shared_ptr<Connection<Multiplex>>> _clients;
    std::shared_ptr<Multiplex> _multiplex;
public:
    SubReactor() {
        static int id = 0;
        _multiplex = std::make_shared<Multiplex>(
            [this](int x) {this->read_callback(x);},
            [this](int x) {this->write_callback(x);}, "sub reactor " + std::to_string(id));
        id++;
    }

    void disconnect(int fd) {
        printf("Goodbye %s : %d\n",
               inet_ntoa(_clients[fd]->addr().sin_addr),
               ntohs(_clients[fd]->addr().sin_port));

        _clients.erase(fd);
    }


    void write_callback(int fd) {
        if(_clients.find(fd) == _clients.end()) return;
        int n = _clients[fd]->send_http();
        //printf("%d send len(%d) message\n", fd, n);
        if(n < 0) {
            disconnect(fd);
            printf("send error: %d\n", fd);
        }
    }

    void read_callback(int fd) {
        if(_clients.find(fd) == _clients.end()) return;
        int n = _clients[fd]->recv_http();
        //int n = 10;
        printf("%d recv len(%d) message\n", fd, n);
        if(n <= 0) {
            disconnect(fd);
            if(n == -1) {
                printf("recv error: %d\n", fd);
                return;
            }
        }
        return;
    }

    void connect(const std::shared_ptr<Socket>& sock, struct sockaddr_in client) {
        _clients[sock->fd()] = std::make_shared<Connection<Multiplex>>(sock, client, _multiplex);
        _clients[sock->fd()]->sock()->sock_nonblock();
        printf("i recv a tcp connection: %d!\n", sock->fd());
    }

    void start() {
        printf("sub reactor start!!!!\n");
        _multiplex->dispatch();
    }
};
#endif //WEBSERVER_SUB_REACTOR_H
