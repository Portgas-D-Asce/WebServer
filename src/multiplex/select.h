#ifndef WEBSERVER_SELECT_H
#define WEBSERVER_SELECT_H
#include <sys/select.h>
#include <string>
#include <memory>
#include <functional>

class Select {
private:
    fd_set _fds;
    int _mx;
    std::function<void(int)> _read_callback;
    std::function<void(int)> _write_callback;
    std::string _name;
public:
    const static int MX = 1024;
public:
    explicit Select(std::function<void(int)> read_callback,
                    std::function<void(int)> write_callback, std::string s)
        : _mx(0), _read_callback(read_callback), _write_callback(write_callback), _name(s) {
        FD_ZERO(&_fds);
    }

    // main reactor call this add listen_fd when constructing
    // sub reactor call this add client_fd when add new connection
    void add(int fd) {
        printf("%s pay attention: %d\n", _name.c_str(), fd);
        //加入关注列表
        FD_SET(fd, &_fds);
        //更新最大文件描述符
        if(fd > _mx) _mx = fd;
    }

    // sub reactor call this rm client_fd when rm a connection
    void rm(int fd) {
        printf("%s remove attention: %d\n", _name.c_str(), fd);
        FD_CLR(fd, &_fds);
        // 小 bug 也要注意，_mx 不要越界
        while(_mx > 0 && !FD_ISSET(_mx, &_fds)) {
            _mx--;
        }
    }

    bool check_mx(int fd) {
        return fd < MX;
    }

    void dispatch() {
        while(true) {
            fd_set rfds = _fds, wfds = _fds;
            timeval tv{};
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            // 设置为阻塞模式会导致子 select 无法检测到就绪 fd
            //int cnt = select(_mx + 1, &fds, NULL, NULL, NULL);
            int cnt = select(_mx + 1, &rfds, &wfds, NULL, &tv);
            if(cnt < 0) {
                perror("select error");
                exit(1);
            }
            // 打印语句注释了，会概率性失败？？？？？？？？
            // interrupted by signal 11: SIGSEGV
            //printf("%s: %d\n", _name.c_str(), cnt);

            // cnt is event's cnt, not fd's cnt
            for(int i = 0; cnt > 0; ++i) {
                if(FD_ISSET(i, &rfds)) {
                    cnt--;
                    _read_callback(i);
                }
                if(FD_ISSET(i, &wfds)) {
                    cnt--;
                    // i maybe removed in read_callback
                    if(FD_ISSET(i, &_fds)) {
                        _write_callback(i);
                    }
                }
            }
        }
    }
};

#endif //WEBSERVER_SELECT_H
