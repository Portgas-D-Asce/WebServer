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
    // mutable std::mutex _mtx;
    mutable std::mutex _event_mtx;
public:
    explicit SubReactor(int id) {
        auto read_callback = std::bind(&SubReactor::read_callback, this, std::placeholders::_1);
        auto write_callback = std::bind(&SubReactor::write_callback, this, std::placeholders::_1);
        std::string name = Config::SUB_REACTOR_NAME + " " + std::to_string(id);
        _multiplex = std::make_shared<Multiplex>(read_callback, write_callback, name);
    }

    void event_callback(const char* ptr, size_t len, int fd, long long id) const {
        std::lock_guard<std::mutex> lg(_event_mtx);
        // connection not exist
        if(!_clients[fd] || _clients[fd]->id() != id) {
            return;
        }

        _clients[fd]->callback(ptr, len);

        // 新数据到来，重新激活写事件
        _multiplex->mod(fd);
    }

    // main reactor thread
    void connect(const std::shared_ptr<Socket>& sock) {
        int fd = sock->fd();
        // 最大文件描述符限制
        if(fd >= Multiplex::MX) {
            printf("too large fd: %d, refuse connection\n", sock->fd());
            // 不再使用 RAII 后, 此处也要手动释放 fd
            close(fd);
            return;
        }

        // {
        //     //printf("Welcome\n");
        //     std::lock_guard<std::mutex> lg(_mtx);

        //     // not only conflict with disconnect but also with write_callback and read_callback
        //     // when connection is constructed, but _clients[fd] is nullptr
        //     // fd has been "up tree", so select/poll/epoll can receive even
        //     // when use _clients[fd] in write_callback and read_callback will segment fault
        //     _clients[fd] = std::make_shared<Connection>(sock);

        //     // to avoid using mutex in write_callback and read_callback,
        //     // so move it here from connection's constructor
        //     // can it move out of the lock？？？
        //     _multiplex->add(fd);
        // }

        // 1 和 2 顺序不能调换, 否则添加到监听树后, 立马收到读写事件, 却发现 connection 还没构造好
        // 1、先创建 connection
        _clients[fd] = std::make_shared<Connection>(sock);

        // 2、将 fd 添加到监听树
        _multiplex->add(fd);

    }

    // sub reactor thread
    void disconnect(int fd) {
        //_clients.erase(fd);
        // std::lock_guard<std::mutex> lg(_mtx);

        // to avoid using mutex in write_callback and read_callback,
        // so move it here from connection's constructor
        // can it move out of the lock？？？
        // _multiplex->rm(fd);

        // std::lock_guard<std::mutex> lg_event(_event_mtx);
        // release tcp connection
        // after close fd and before _clients[fd] = nullptr
        // fd maybe get by connection immediately: connection's fd == disconnection's fd
        // _clients[fd] = nullptr;

        //printf("Goodbye\n");

        // 1 和 2 顺序不能调换, 否则会导致监听到了事件，但是 connection 已经释放了, 导致 core dump
        // 2 和 3 顺序不能调换, 先 close fd, connection accept 立刻获得 fd 并初始化 connection, 与 2 释放 connection 冲突
        std::lock_guard<std::mutex> lg_event(_event_mtx);

        // 1、从监听树上移除 fd
        _multiplex->rm(fd);

        // 2、释放 connection,
        _clients[fd] = nullptr;

        // 3、关闭文件描述符
        close(fd);
    }

    // sub reactor thread
    void write_callback(int fd) {
        if(!_clients[fd]) return;
        // 写数据发生错误
        if(_clients[fd]->send_http() < 0) {
            disconnect(fd);
            printf("send error: %d\n", fd);
        }
    }

    // sub reactor thread
    void read_callback(int fd) {
        std::vector<std::string> msgs;
        // 收到断开连接请求/接收错误
        if(_clients[fd]->recv_http(msgs) <= 0) {
            disconnect(fd);
            return;
        }

        ThreadPool& pool = ThreadPool::get_instance();
        for(auto &msg : msgs) {
            long long id = _clients[fd]->id();
            pool.enqueue(Handler(), std::move(msg), [this, fd, id](const char* ptr, size_t len) {
                this->event_callback(ptr, len, fd, id);
            });
        }
    }

    void start() {
        printf("sub reactor start!!!!\n");
        _multiplex->dispatch();
    }
};
#endif //WEBSERVER_SUB_REACTOR_H
