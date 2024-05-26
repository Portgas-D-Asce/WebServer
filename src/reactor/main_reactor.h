#ifndef WEBSERVER_MAIN_REACTOR_H
#define WEBSERVER_MAIN_REACTOR_H
#include <memory>
#include <vector>
#include <functional>
#include "../common/socket.h"
#include "sub_reactor.h"

template<typename MainMultiplex, typename SubMultiplex>
class MainReactor {
private:
    using SReactor = SubReactor<SubMultiplex>;
    std::shared_ptr<Socket> _sock;
    std::shared_ptr<MainMultiplex> _multiplex;
    std::vector<std::shared_ptr<SReactor>> _sub_reactors;
public:
    MainReactor(const std::shared_ptr<Socket>& sock) : _sock(sock) {
        auto read_callback = std::bind(&MainReactor::read_callback, this, std::placeholders::_1);
        auto write_callback = std::bind(&MainReactor::write_callback, this, std::placeholders::_1);
        _multiplex = std::make_shared<MainMultiplex>(read_callback, write_callback, "main reactor");

        _multiplex->add(_sock->fd());

        // 我艹 构造函数只会调用一次
        //_sub_reactors = std::vector<std::shared_ptr<SReactor>>(2, std::make_shared<SReactor>(_pool));
        int n = 1;
        _sub_reactors = std::vector<std::shared_ptr<SReactor>>(n, std::make_shared<SReactor>());
        for(int i = 0; i < n; ++i) {
            _sub_reactors[i] =  std::make_shared<SReactor>();
        }
    }

    void write_callback(int useless) {
        //do nothing
    }

    void read_callback(int useless) {
        auto fd = _sock->sock_accept();
        if(!fd) {
            perror("add client socket");
            return;
        }

        // 设置 client_fd 为非阻塞模式
        fd->sock_nonblock();

        // 将 client fd 加入到 sub reactor
        static int cnt = 0;
        _sub_reactors[cnt & 1]->connect(fd);
        //cnt++;
    }

    void start() {
        std::vector<std::thread> ts;
        int n = _sub_reactors.size();
        for(int i = 0; i < n; ++i) {
            ts.emplace_back(&SReactor::start, _sub_reactors[i]);
        }

        printf("main reactor start!!!\n");
        _multiplex->dispatch();
        for(int i = 0; i < n; ++i) {
            ts[i].join();
        }
    }
};


#endif //WEBSERVER_MAIN_REACTOR_H
