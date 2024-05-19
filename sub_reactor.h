//
// Created by pk on 2024/5/19.
//

#ifndef WEBSERVER_SUB_REACTOR_H
#define WEBSERVER_SUB_REACTOR_H
#include <memory>
#include <map>
#include <functional>
#include "socket.h"
#include "thread_pool_copy.h"
#include "tcp_connection.h"

template<typename Multiplex>
class SubReactor {
private:
    std::map<int, std::shared_ptr<TcpConnection<Multiplex>>> _clients;
    std::shared_ptr<Multiplex> _multiplex;
    std::shared_ptr<ThreadPool> _pool;
public:
    SubReactor(const std::shared_ptr<ThreadPool>& pool) : _pool(pool) {
        static int id = 0;
        _multiplex = std::make_shared<Multiplex>([this](int x) {
            this->callback(x);

        }, "sub reactor " + std::to_string(id));
        id++;
    }

    void disconnect(int fd) {
        printf("Goodbye %s : %d\n",
               inet_ntoa(_clients[fd]->addr().sin_addr),
               ntohs(_clients[fd]->addr().sin_port));

        _clients.erase(fd);
    }

    void callback(int fd) {
        int n = _clients[fd]->recv();
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
        _clients[sock->fd()] = std::make_shared<TcpConnection<Multiplex>>(sock, client, _multiplex, _pool);
        _clients[sock->fd()]->sock()->sock_nonblock();
        printf("i recv a tcp connection: %d!\n", sock->fd());
    }

    void start() {
        printf("sub reactor start!!!!\n");
        _multiplex->dispatch();
    }
};
#endif //WEBSERVER_SUB_REACTOR_H
