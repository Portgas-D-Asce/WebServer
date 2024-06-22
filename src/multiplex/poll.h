#ifndef WEBSERVER_POLL_H
#define WEBSERVER_POLL_H
#include <sys/poll.h>
#include <map>
#include <functional>
#include <sys/poll.h>

class Poll {
public:
    const static int MX = 2048;
private:
    struct pollfd _fds[Poll::MX];
    int _mx;
    bool _flag;
    std::function<void(int)> _read_callback;
    std::function<void(int)> _write_callback;
    std::string _name;
public:
    explicit Poll(std::function<void(int)> read_callback,
                  std::function<void(int)> write_callback,
                  std::string name)
        : _read_callback(read_callback),
        _write_callback(write_callback), _name(name) {
        _mx = 0;
        _flag = 0;
        for(int i = 0; i < MX; ++i) {
            _fds[i].fd = -1;
        }
    }

    void add(int fd) {
        _fds[_mx].fd = fd;
        _fds[_mx].events = POLLRDNORM | POLLWRNORM;
        _mx++;
        printf("%s add attention: %d\n", _name.c_str(), fd);
    }

    void mod(int fd) {
        // do nothing
        //printf("%s mod attention: %d\n", _name.c_str(), fd);
    }

    void rm(int fd) {
        _flag = true;
        printf("%s remove attention: %d\n", _name.c_str(), fd);
    }

    void dispatch() {
        while(true) {
            int cnt = poll(_fds, _mx, 1000);
            if(cnt < 0) {
                perror("poll error");
                exit(1);
            }

            for(int i = 0; cnt > 0; ++i) {
                _flag = false;
                int temp = 0;
                if(_fds[i].revents & POLLRDNORM) {
                    temp = 1;
                    _read_callback(_fds[i].fd);
                }

                if(_fds[i].revents & POLLWRNORM) {
                    temp = 1;
                    _write_callback(_fds[i].fd);
                }
                cnt -= temp;
                if(_flag) {
                    _mx--;
                    _fds[i] = _fds[_mx];
                    --i;
                }
            }
        }
    }
};
#endif //WEBSERVER_POLL_H
