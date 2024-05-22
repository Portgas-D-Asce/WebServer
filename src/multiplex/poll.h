#ifndef WEBSERVER_POLL_H
#define WEBSERVER_POLL_H
#include <sys/poll.h>
#include <map>
#include <functional>
#include <sys/poll.h>

class Poll {
public:
    const static int MX = 10240;
private:
    struct pollfd _fds[Poll::MX];
    std::function<void(int)> _read_callback;
    std::function<void(int)> _write_callback;
    std::string _name;
public:
    explicit Poll(std::function<void(int)> read_callback,
                  std::function<void(int)> write_callback,
                  std::string name)
        : _read_callback(read_callback),
        _write_callback(write_callback), _name(name) {
        for(int i = 0; i < MX; ++i) {
            _fds[i].fd = -1;
        }
    }

    void add(int fd) {
        _fds[fd].fd = fd;
        _fds[fd].events = POLLRDNORM | POLLWRNORM;
        printf("pay attention: %d\n", fd);
    }

    void rm(int fd) {
        _fds[fd].fd = -1;
        printf("remove attention: %d\n", fd);
    }

    void dispatch() {
        while(true) {
            struct pollfd temp[Poll::MX];
            for(int i = 0; i < MX; ++i) {
                temp[i] = _fds[i];
            }
            int cnt = poll(temp, MX, 0);
            if(cnt < 0) {
                perror("poll error");
                exit(1);
            }

            //printf("%s: %d\n", _name.c_str(), cnt);

            for(int i = 0; cnt > 0 && i < MX; ++i) {
                if(temp[i].revents & POLLRDNORM) {
                    cnt--;
                    _read_callback(_fds[i].fd);
                }

                if(temp[i].revents & POLLWRNORM) {
                    cnt--;
                    _write_callback(_fds[i].fd);
                }
                //cnt -= flag;
            }
        }
    }
};
#endif //WEBSERVER_POLL_H
