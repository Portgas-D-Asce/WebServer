#include <sys/poll.h>
#include <map>
#include <functional>
#include "../tcp/tcp_server.h"

#ifndef WEBSERVER_POLL_H
#define WEBSERVER_POLL_H

class Poll {
public:
    const static int MX = 1024;
private:
    struct pollfd _fds[Poll::MX];
    int _mx;
    bool _rm_flag;
    std::function<void(int)> _read_callback;
    std::function<void(int)> _write_callback;
    std::string _name;

public:
    explicit Poll(std::function<void(int)> read_callback,
                  std::function<void(int)> write_callback,
                  std::string name)
        : _mx(0),  _read_callback(read_callback),
        _write_callback(write_callback), _name(name) {}

    void add(int fd) {
        _fds[_mx].fd = fd;
        _fds[_mx].events = POLLRDNORM | POLLWRNORM;
        _mx++;
        printf("pay attention: %d\n", fd);
    }

    void rm(int fd) {
        _rm_flag = true;
        printf("remove attention: %d\n", fd);
    }

    bool check_mx(int useless) {
        return _mx < MX;
    }

    void dispatch() {
        while(true) {
            int cnt = poll(_fds, _mx, -1);
            if(cnt < 0) {
                perror("poll error");
                exit(1);
            }

            for(int i = 0; cnt > 0; ++i) {
                bool flag = false;
                _rm_flag = false;
                if(_fds[i].revents & POLLRDNORM) {
                    flag = true;
                    _read_callback(_fds[i].fd);
                }

                if(_fds[i].revents & POLLWRNORM) {
                    flag = true;
                    if(!_rm_flag) {
                        _write_callback(_fds[i].fd);
                    }
                }
                cnt -= flag;

                if(_rm_flag) {
                    _mx--;
                    _fds[i] = _fds[_mx];
                    i--;
                }
            }
        }
    }
};
#endif //WEBSERVER_POLL_H
