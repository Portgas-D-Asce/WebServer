#ifndef WEBSERVER_EPOLL_H
#define WEBSERVER_EPOLL_H
#include <sys/epoll.h>

class EPoll {
public:
    const static int MX = 65535;
private:
    struct epoll_event _evs[MX];
    int _fd;
    std::function<void(int)> _read_callback;
    std::function<void(int)> _write_callback;
    std::string _name;
public:
    explicit EPoll(std::function<void(int)> read_callback, std::function<void(int)> write_callback, std::string name)
        : _read_callback(read_callback), _write_callback(write_callback), _name(name) {
		_fd = epoll_create1(0);
		if(_fd == -1) {
			perror("epoll_creat1");
			exit(1);
		}
	}

    void add(int fd) {
	    struct epoll_event ev;
	    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
	    ev.data.fd = fd;
	    if(epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		    perror("epoll_ctl_add");
		    exit(1);
	    }
        printf("pay attention: %d\n", fd);
    }

    void rm(int fd) {
	// do nothing, becasue it has been down when we close fd
	// you can find it in man epoll Q6
	
	    if(epoll_ctl(_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
		    perror("epoll_ctl_add");
		    exit(1);
	    }
        printf("remove attention: %d\n", fd);
    }

    void dispatch() {
        while(true) {
            int cnt = epoll_wait(_fd, _evs, sizeof(_evs), 0);
            if(cnt < 0) {
                perror("epoll wait");
                exit(1);
            }

	    //printf("%s recv %d\n", _name.c_str(), cnt);

            for(int i = 0; i < cnt && i < MX; ++i) {
		    int fd = _evs[i].data.fd;
                if(_evs[i].events & EPOLLIN) {
                    _read_callback(_evs[i].data.fd); 
                }

                if(_evs[i].events & EPOLLOUT) {
                    _write_callback(_evs[i].data.fd);
                }
            }
        }
    }
};


#endif //WEBSERVER_EPOLL_H