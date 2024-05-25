#ifndef WEBSERVER_SUB_REACTOR_H
#define WEBSERVER_SUB_REACTOR_H
#include <memory>
//#include <map>
#include <vector>
#include <functional>
#include "../common/socket.h"
#include "../common/connection.h"

template<typename Multiplex>
class SubReactor {
private:
    //std::map<int, std::shared_ptr<Connection<Multiplex>>> _clients;
    std::shared_ptr<Connection<Multiplex>> _clients[Multiplex::MX];
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
            printf("too many fd: %d, refuse connection\n", sock->fd());
            return;
        }

        {

            //printf("Welcome\n");
            std::lock_guard<std::mutex> lg(_mtx);
            // not only conflict with disconnect but also with write_callback and read_callback
            // when connection is constructed, but _clients[fd] is nullptr
            // fd has been "up tree", so select/poll/epoll can receive even
            // when use _clients[fd] in write_callback and read_callback will segment fault
            _clients[fd] = std::make_shared<Connection<Multiplex>>(sock, _multiplex);

            // to avoid using mutex in write_callback and read_callback,
            // so move it here from connection's constructor
            _multiplex->add(fd);
        }
    }

    // sub reactor thread
    void disconnect(int fd) {
        //_clients.erase(fd);
        std::lock_guard<std::mutex> lg(_mtx);

        // to avoid using mutex in write_callback and read_callback,
        // so move it here from connection's constructor
        _multiplex->rm(fd);

        // release tcp connection
        // after close fd and before _clients[fd] = nullptr
        // fd maybe get by connection immediately: connection's fd == disconnection's fd
        _clients[fd] = nullptr;

        //printf("Goodbye\n");
    }

    // sub reactor thread
    void write_callback(int fd) {
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
        int n = _clients[fd]->recv_http();
        //printf("%d recv len(%d) message\n", fd, n);
        if(n <= 0) {
            disconnect(fd);
            if(n == -1) {
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
