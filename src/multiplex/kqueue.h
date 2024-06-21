/*
 * kqueue:
 * * 读/写事件需要单独 ”上树“, 无法一块 ”上树“
 * * 事件触发时, 一个 event 要么是读事件, 要么是写事件
 * * 支持读/写采用不同的模式, 读水平触发, 写边缘触发
 */
#ifndef WEBSERVER_KQUEUE_H
#define WEBSERVER_KQUEUE_H
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <functional>

class KQueue {
public:
    const static int MX = 65535;
private:
    int _fd;
    struct kevent _evs[1024];
    std::function<void(int)> _read_callback;
    std::function<void(int)> _write_callback;
    std::string _name;
public:
    explicit KQueue(std::function<void(int)> read_callback, std::function<void(int)> write_callback, std::string name)
    : _read_callback(read_callback), _write_callback(write_callback), _name(name) {
        _fd = kqueue();
        if(_fd == -1) {
            perror("kqueue");
            exit(1);
        }
    }

    void add(int fd) {
        struct kevent ev;
        // 读事件 水平触发方式上树
        EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if(kevent(_fd, &ev, 1, NULL, 0, NULL) == -1) {
            perror("kevent");
            exit(1);
        }
        // 写事件 边缘触发方式上树
        EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, NULL);
        if(kevent(_fd, &ev, 1, NULL, 0, NULL) == -1) {
            perror("kevent");
            exit(1);
        }
        printf("%s pay attention: %d\n", _name.c_str(), fd);
    }

    void mod(int fd) {
        struct kevent ev;
        // 写事件 重新边缘模式上树, 为了唤醒写事件
        EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, NULL);
        if(kevent(_fd, &ev, 1, NULL, 0, NULL) == -1) {
            perror("kevent");
            exit(1);
        }
        printf("%s mod attention: %d\n", _name.c_str(), fd);
    }

    void rm(int fd) {
        struct kevent ev;
        // 读事件 下树
        EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        if(kevent(_fd, &ev, 1, NULL, 0, NULL) == -1) {
            perror("kevent");
            exit(1);
        }
        // 写事件 下树
        EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        if(kevent(_fd, &ev, 1, NULL, 0, NULL) == -1) {
            perror("kevent");
            exit(1);
        }
        printf("%s remove attention: %d\n", _name.c_str(), fd);
    }

    void dispatch() {
        struct timespec tt{1, 0};
        while(true) {
            int cnt = kevent(_fd, NULL, 0, _evs, 1024, &tt);
            if(cnt < 0) {
                perror("kevent");
                exit(1);
            }

            for(int i = 0; i < cnt; ++i) {
                if(_evs[i].filter == EVFILT_READ) {
                    // printf("%s xxx\n", _name.c_str());
                    _read_callback(_evs[i].ident);
                } else if(_evs[i].filter == EVFILT_WRITE) {
                    // printf("%s yyy\n", _name.c_str());
                    _write_callback(_evs[i].ident);
                }
            }
        }
    }
};

#endif //WEBSERVER_KQUEUE_H
