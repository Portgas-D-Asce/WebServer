//
// Created by pk on 2024/5/15.
//
#include <sys/poll.h>
#include <map>
#include <functional>
#include "../tcp/tcp_server.h"

#ifndef WEBSERVER_POLL_H
#define WEBSERVER_POLL_H

static const int MAX_EVENTS = 1024;
class Poll {
private:
    struct pollfd _fds[MAX_EVENTS];
    std::map<int, int> mp;
    int _mx;
    std::function<void(int)> _callback;
public:
    explicit Poll(std::function<void(int)> callback)
        : _mx(0),  _callback(callback) {}

    void add(int fd) {
        mp[fd] = _mx;
        _fds[_mx].fd = fd;
        _fds[_mx].events = POLLRDNORM;
        _mx++;
        printf("pay attention: %d\n", fd);
    }

    void rm(int fd) {
        // 获取关闭套接字的位置
        int idx = mp[fd];
        // 将关闭套接字的位置移除
        mp.erase(fd);

        _mx--;
        //最后一个关注项放到当前位置
        _fds[idx] = _fds[_mx];
        mp[_fds[idx].fd] = idx;
        printf("remove attention: %d\n", fd);
    }

    void dispatch() {
        int server_fd = 0;
        while(true) {
            int cnt = poll(_fds, _mx, -1);
            if(cnt < 0) {
                perror("poll error");
                exit(1);
            }

            printf("xxxxx\n");

            for(int i = 0; cnt > 0; ++i) {
                if (!(_fds[i].revents & POLLRDNORM)) continue;
                cnt--;
                printf("zzzz: %d\n", i);
                _callback(_fds[i].fd);

                //if (_fds[i].fd == server_fd) {
                    //_server.connect();
                //} else {
                    //int flag = _server.recv_msg(_fds[i].fd);
                    //if(flag == 0) {
                    //    i--;
                    //}
                //}
            }
        }
    }
};
#endif //WEBSERVER_POLL_H
