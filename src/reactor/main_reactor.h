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
    MainReactor(const std::shared_ptr<Socket>& sock) :_sock(sock) {
        /*
         * error: no viable conversion from '__bind<void (MainReactor<Select, Select>::*)(int),
         * MainReactor<Select, Select> *>' to 'std::function<void (int)>'
        std::function<void(int)> func = std::bind(&MainReactor::callback, this);
        _multiplex = std::make_shared<MainMultiplex>(func, "main reactor");
         */
        _multiplex = std::make_shared<MainMultiplex>(
            [this](int x) {this->read_callback(x);},
            [this](int x) {this->write_callback(x);}, "main reactor");
        _multiplex->add(_sock->fd());

        // 我艹 构造函数只会调用一次
        //_sub_reactors = std::vector<std::shared_ptr<SReactor>>(2, std::make_shared<SReactor>(_pool));
        _sub_reactors = std::vector<std::shared_ptr<SReactor>>(2);
        _sub_reactors[0] =  std::make_shared<SReactor>();
        _sub_reactors[1] =  std::make_shared<SReactor>();
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

        static int cnt = 0;
        _sub_reactors[cnt & 1]->connect(fd);
        cnt++;
    }

    void start() {
        std::vector<std::thread> ts;
        int n = _sub_reactors.size();
        for(int i = 0; i < n; ++i) {
            ts.emplace_back(&SReactor::start, _sub_reactors[i]);
        }

        _multiplex->dispatch();
        printf("main reactor start!!!\n");
        for(int i = 0; i < n; ++i) {
            ts[i].join();
        }
    }
};


#endif //WEBSERVER_MAIN_REACTOR_H
