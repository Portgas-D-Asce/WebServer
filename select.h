//
// Created by pk on 2024/5/15.
//

#ifndef WEBSERVER_SELECT_H
#define WEBSERVER_SELECT_H
#include <sys/select.h>
#include <memory>
#include "tcp_server.h"

class Select {
private:
    TcpServer<Select>& _server;
    fd_set _fds;
    int _mx;
public:
    explicit Select(TcpServer<Select>& server) : _server(server), _mx(-1) {
        FD_ZERO(&_fds);
        add(_server.listen_fd());
    }

    void add(int fd) {
        printf("pay attention: %d\n", fd);
        //加入关注列表
        FD_SET(fd, &_fds);
        //更新最大文件描述符
        if(fd > _mx) _mx = fd;
    }

    void rm(int fd) {
        printf("remove attention: %d\n", fd);
        FD_CLR(fd, &_fds);
        while(!FD_ISSET(_mx, &_fds)) {
            _mx--;
        }
    }

    void dispatch() {
        int server_fd = _server.listen_fd();
        while(true) {
            fd_set fds = _fds;
            //开始监视文件描述符，最后一个 NULL 表示不设置超时时间
            int cnt = select(_mx + 1, &fds, NULL, NULL, NULL);
            if(cnt < 0) {
                perror("select error");
                exit(1);
            }

            //逐个检查文件描述符，找到已经准备好的文件描述符
            for(int i = 0; cnt > 0; ++i) {
                if(!FD_ISSET(i, &fds)) continue;
                cnt--;
                //新的连接请求到达哦
                if(i == server_fd) {
                    _server.connect();
                } else {
                    _server.recv_msg(i);
                }
            }
        }
    }
};

#endif //WEBSERVER_SELECT_H
