#ifndef WEBSERVER_SUB_REACTOR_H
#define WEBSERVER_SUB_REACTOR_H
#include <memory>
//#include <map>
#include <vector>
#include <functional>
#include "../common/socket.h"
#include "../common/connection.h"
#include "../handler/handler.h"
#include "../thread/thread_pool.h"
#include "../config/config.h"

template<typename Multiplex>
class SubReactor {
private:
    //std::map<int, std::shared_ptr<Connection<Multiplex>>> _clients;
    std::shared_ptr<Connection> _clients[Multiplex::MX];
    std::shared_ptr<Multiplex> _multiplex;
    // a mutex is necessary:
    // main reactor thread insert connection into _client
    // sub reactor thread erase connection from _client
    mutable std::mutex _mtx;
    mutable std::mutex _event_mtx;
public:
    SubReactor(int id) {
        auto read_callback = std::bind(&SubReactor::read_callback, this, std::placeholders::_1);
        auto write_callback = std::bind(&SubReactor::write_callback, this, std::placeholders::_1);
        std::string name = Config::SUB_REACTOR_NAME + " " + std::to_string(id);
        _multiplex = std::make_shared<Multiplex>(read_callback, write_callback, name);
    }

    void event_callback(const std::string& msg, int fd, long long id) const {
        std::lock_guard<std::mutex> lg(_event_mtx);
        // connection not exist
        if(!_clients[fd] || _clients[fd]->id() != id) {
            return;
        }

        _clients[fd]->callback(msg);

        // 新数据到来，重新激活写事件
        _multiplex->mod(fd);
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
            _clients[fd] = std::make_shared<Connection>(sock);

            // to avoid using mutex in write_callback and read_callback,
            // so move it here from connection's constructor
            // can it move out of the lock？？？
            _multiplex->add(fd);
        }
    }

    // sub reactor thread
    void disconnect(int fd) {
        //_clients.erase(fd);
        std::lock_guard<std::mutex> lg(_mtx);

        // to avoid using mutex in write_callback and read_callback,
        // so move it here from connection's constructor
        // can it move out of the lock？？？
        _multiplex->rm(fd);

        std::lock_guard<std::mutex> lg_event(_event_mtx);
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
        std::vector<std::string> msgs;
        int n = _clients[fd]->recv_http(msgs);
        //printf("%d recv len(%d) message\n", fd, n);
        if(n <= 0) {
            disconnect(fd);
            if(n == -1) {
                return;
            }
        }

        ThreadPool& pool = ThreadPool::get_instance();
        for(auto &msg : msgs) {
            long long id = _clients[fd]->id();
            pool.enqueue(Handler(), std::move(msg), [this, fd, id](const std::string& resp) {
                this->event_callback(resp, fd, id);
            });
        }
    }

    void start() {
        printf("sub reactor start!!!!\n");
        _multiplex->dispatch();
    }
};
#endif //WEBSERVER_SUB_REACTOR_H
