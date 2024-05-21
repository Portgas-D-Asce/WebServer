#ifndef WEBSERVER_SOCKET_H
#define WEBSERVER_SOCKET_H
#include <string>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

class Socket {
private:
    int _fd;
private:
    explicit Socket(int fd) {
        _fd = fd;
    }
public:
    Socket() {
        _fd = socket(AF_INET, SOCK_STREAM, 0);
    }

    ~Socket() {
        static int cnt = 0;
        cnt++;
        printf("%d, fd: %d will be closed!\n", cnt, _fd);
        close(_fd);
    }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    static struct sockaddr_in sock_address(const std::string& ip, int port) {
        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        return addr;
    }

    void sock_bind(const struct sockaddr_in& addr) const {
        if(bind(_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("bind");
            exit(1);
        }
    }

    void sock_listen(int cnt) const {
        if(listen(_fd, cnt) == -1) {
            perror("listen");
        }
    }

    void sock_connect(const struct sockaddr_in& addr) const {
        if(connect(_fd, (struct sockaddr *)&(addr), sizeof(addr)) == -1) {
            perror("connect");
        }
    }

    auto sock_accept(struct sockaddr_in& addr) const {
        socklen_t len = sizeof(addr);
        int fd = accept(_fd, (struct sockaddr *)&(addr), &len);
        if(fd == - 1) {
            perror("accept");
            return std::shared_ptr<Socket>();
        }
        return std::shared_ptr<Socket>(new Socket(fd));
    }

    void sock_nonblock() const {
        int flags = fcntl(_fd, F_GETFL, 0);
        fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
    }

    int sock_send(const char* buf, int sz) const {
        int temp = sz;
        while(sz > 0) {
            int cnt = send(_fd, (void *)buf, sz, 0);
            if(cnt == -1) {
                if(errno == 35) {
                    return temp - sz;
                }
                perror("send");
                printf("errno = %d\n", errno);
                return -1;
            }
            sz -= cnt;
            buf += cnt;
        }
        return temp;
    }

    int sock_recv(char *buf, int sz) const {
        long long temp = sz;
        while(sz > 0) {
            int cnt = recv(_fd, buf, sz, 0);
            if(cnt == -1) {
                // 数据读完了
                if(errno == 35) {
                    return temp - sz;
                }
                perror("recv");
                printf("errno = %d\n", errno);
                return -1;
            }
            if(cnt == 0) {
                printf("recv close\n");
                return 0;
            }
            sz -= cnt;
            buf += cnt;
        }
        return temp;
    }

    int fd() const {
        return _fd;
    }

};


#endif //WEBSERVER_SOCKET_H
