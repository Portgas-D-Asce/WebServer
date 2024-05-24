#ifndef WEBSERVER_SUB_REACTOR_H
#define WEBSERVER_SUB_REACTOR_H
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include "../common/socket.h"
#include "../common/connection.h"

template<typename Multiplex>
class SubReactor {
private:
    std::map<int, std::shared_ptr<Connection<Multiplex>>> _clients;
    std::shared_ptr<Multiplex> _multiplex;
    // a mutex is necessary:
    // main reactor thread insert connection into _client
    // sub reactor thread erase connection from _client
    std::mutex _mtx;
public:
    SubReactor() {
        static int id = 0;
        auto read_callback = std::bind(&SubReactor::read_callback, this, std::placeholders::_1);
        auto write_callback = std::bind(&SubReactor::write_callback, this, std::placeholders::_1);
        std::string name = "sub reactor " + std::to_string(id);
        _multiplex = std::make_shared<Multiplex>(read_callback, write_callback, name);
        id++;
    }

    // main reactor thread
    void connect(const std::shared_ptr<Socket>& sock) {
        int fd = sock->fd();
        // 最大文件描述符限制
        if(fd >= Multiplex::MX) {
            //printf("cur connection num: %d\n", _clients.size());
            printf("too many fd: %d, refuse connection\n", sock->fd());
            return;
        }

        {
            std::unique_lock<std::mutex> ul(_mtx);
            _clients[fd] = std::make_shared<Connection<Multiplex>>(sock, _multiplex);
        }
    }

    // sub reactor thread
    void disconnect(int fd) {
        _clients.erase(fd);
        printf("Goodbye\n");
    }

    // sub reactor thread
    void write_callback(int fd) {
    std::unique_lock<std::mutex> ul(_mtx);
	if(!_clients[fd]) return;
        int n = _clients[fd]->send_http();
        //printf("%d send len(%d) message\n", fd, n);
        if(n < 0) {
            disconnect(fd);
            printf("send error: %d\n", fd);
        }
    }

    // sub reactor thread
    void read_callback(int fd) {
        std::unique_lock<std::mutex> ul(_mtx);
        int n = _clients[fd]->recv_http();
        //printf("%d recv len(%d) message\n", fd, n);
        if(n <= 0) {
            disconnect(fd);
            if(n == -1) {
                printf("recv error: %d\n", fd);
                return;
            }
        }
    }

    void start() {
        printf("sub reactor start!!!!\n");
        _multiplex->dispatch();
    }
};
#endif //WEBSERVER_SUB_REACTOR_H
