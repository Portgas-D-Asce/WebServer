//
// Created by pk on 2024/5/19.
//

#ifndef WEBSERVER_MAIN_REACTOR_H
#define WEBSERVER_MAIN_REACTOR_H
#include <memory>
#include <vector>
#include <functional>
#include "socket.h"
#include "thread_pool_copy.h"
#include "sub_reactor.h"

template<typename MainMultiplex, typename SubMultiplex>
class MainReactor {
private:
    using SReactor = SubReactor<SubMultiplex>;
    std::shared_ptr<Socket> _sock;
    std::shared_ptr<MainMultiplex> _multiplex;
    std::shared_ptr<ThreadPool> _pool;
    std::vector<std::shared_ptr<SReactor>> _sub_reactors;
public:
    MainReactor(const std::shared_ptr<Socket>& sock)
        :_sock(sock), _pool(std::make_shared<ThreadPool>(4)) {
        /*
         * error: no viable conversion from '__bind<void (MainReactor<Select, Select>::*)(int),
         * MainReactor<Select, Select> *>' to 'std::function<void (int)>'
        std::function<void(int)> func = std::bind(&MainReactor::callback, this);
        _multiplex = std::make_shared<MainMultiplex>(func, "main reactor");
         */
        _multiplex = std::make_shared<MainMultiplex>([this](int x) {
            this->callback(x);
        }, "main reactor");
        _multiplex->add(_sock->fd());

        // 我艹 构造函数只会调用一次
        //_sub_reactors = std::vector<std::shared_ptr<SReactor>>(2, std::make_shared<SReactor>(_pool));

        _sub_reactors = std::vector<std::shared_ptr<SReactor>>(2);
        _sub_reactors[0] =  std::make_shared<SReactor>(_pool);
        _sub_reactors[1] =  std::make_shared<SReactor>(_pool);
    }


    void callback(int useless) {
        struct sockaddr_in client{};
        auto fd = _sock->sock_accept(client);
        if(!fd) {
            perror("add client socket");
            return;
        }
        //_multiplex->add(fd->fd());
        printf("Welcome home %s : %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        static int cnt = 0;
        _sub_reactors[cnt & 1]->connect(fd, client);
        cnt++;
    }

    void start() {
        std::vector<std::thread> ts;
        int n = _sub_reactors.size();
        for(int i = 0; i < n; ++i) {
            ts.emplace_back(&SReactor::start, _sub_reactors[i]);
            ts[i].detach();
        }

        printf("main reactor start!!!\n");
        _multiplex->dispatch();
    }

};


#endif //WEBSERVER_MAIN_REACTOR_H
