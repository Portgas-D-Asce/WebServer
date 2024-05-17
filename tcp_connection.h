//
// Created by pk on 2024/5/16.
//

#ifndef WEBSERVER_TCP_CONNECTION_H
#define WEBSERVER_TCP_CONNECTION_H
#include <arpa/inet.h>
#include <memory>
#include "socket.h"
#include "thread_pool.h"
#include "handler.h"

template<typename Multiplex>
class TcpConnection {
private:
    std::shared_ptr<Socket> _sock;
    std::shared_ptr<Multiplex> _multiplex;
    std::shared_ptr<ThreadPool> _pool;
    struct sockaddr_in _addr;
    int _status;
    int _sz;
    int _cur;
    char _in_buf[1024 * 1024];
    char _out_buf[1024 * 1024];

public:
    TcpConnection() {};
    TcpConnection(const std::shared_ptr<Socket>& fd,
        const struct sockaddr_in& addr,
        const std::shared_ptr<Multiplex>& multiplex,
        const std::shared_ptr<ThreadPool>& pool)
        : _sock(fd), _addr(addr), _multiplex(multiplex), _pool(pool) {
        _multiplex->add(_sock->fd());
        _status = 0;
        _sz = 0;
        _cur = 0;
    }

    ~TcpConnection() {
        _multiplex->rm(_sock->fd());
    }

    std::shared_ptr<Socket>& sock() {
        return _sock;
    }

    struct sockaddr_in& addr() {
        return _addr;
    }

    int recv() {
        int total = 0;
        while(true) {
            if(_status == 0) {
                //int type = 0;
                //int cnt = _sock->sock_recv((char *)&type, sizeof(type));
                //if(cnt <= 0) return cnt;
                //type = ntohl(type);
                int need = 4 - _cur;
                int cnt = _sock->sock_recv(_in_buf + _cur, need);
                if(cnt == -1) {
                    _reset();
                    return -1;
                }
                if(cnt == 0) {
                    //_reset();
                    return total;
                }
                total += cnt;
                if(cnt == need) {
                    _status = 1;
                    _cur += cnt;
                    _sz = ntohl(*((int *)_in_buf));
                } else {
                    _cur += cnt;
                    break;
                }
            } else {
                int need = _sz + 4 - _cur;
                int cnt = _sock->sock_recv(_in_buf + _cur, need);
                if(cnt == -1) {
                    _reset();
                    return -1;
                }
                if(cnt == 0) {
                    //_reset();
                    return total;
                }
                total += cnt;
                if(need == cnt) {
                    // 处理消息
                    int type = ntohl(*((int *)(_in_buf + 4)));
                    //printf("message type: %d\n", type);
                    _in_buf[_sz + 4] = '\0';
                    //printf("message: %s\n", _in_buf + 8);
                    std::string s(_in_buf + 8);
                    _pool->enqueue(Handler(), type, s);

                    // 准备下一次接收
                    _reset();
                } else {
                    _cur += cnt;
                    break;
                }

            }
        }
        return total;
    }

private:
    void _reset() {
        _cur = 0;
        _status = 0;
        _sz = 0;
    }
};
#endif //WEBSERVER_TCP_CONNECTION_H
