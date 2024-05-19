//
// Created by pk on 2024/5/15.
//

#ifndef WEBSERVER_SELECT_H
#define WEBSERVER_SELECT_H
#include <sys/select.h>
#include <string>
#include <memory>
#include <functional>
#include "tcp_server.h"

class Select {
private:
    fd_set _fds;
    int _mx;
    std::function<void(int)> _callback;
    std::string _name;
public:
    explicit Select(std::function<void(int)> callback, std::string s)
        : _mx(0), _callback(callback), _name(s) {
        FD_ZERO(&_fds);
    }

    void add(int fd) {
        printf("%s pay attention: %d\n", _name.c_str(), fd);
        //加入关注列表
        FD_SET(fd, &_fds);
        //更新最大文件描述符
        if(fd > _mx) _mx = fd;
    }

    void rm(int fd) {
        printf("%s remove attention: %d\n", _name.c_str(), fd);
        FD_CLR(fd, &_fds);
        // 小 bug 也要注意，_mx 不要越界
        while(_mx > 0 && !FD_ISSET(_mx, &_fds)) {
            _mx--;
        }
    }

    void dispatch() {
        while(true) {
            fd_set fds = _fds;
            timeval tv{};
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            // 设置为阻塞模式会导致子 select 无法检测到就绪 fd
            //int cnt = select(_mx + 1, &fds, NULL, NULL, NULL);
            int cnt = select(_mx + 1, &fds, NULL, NULL, &tv);
            if(cnt < 0) {
                perror("select error");
                exit(1);
            }
            printf("%s: %d\n", _name.c_str(), cnt);

            //逐个检查文件描述符，找到已经准备好的文件描述符
            for(int i = 0; cnt > 0; ++i) {
                if(!FD_ISSET(i, &fds)) continue;
                printf("zzzz: %d\n", i);
                cnt--;
                _callback(i);
            }
        }
    }
};

#endif //WEBSERVER_SELECT_H
