#ifndef WEBSERVER_CONNECTION_H
#define WEBSERVER_CONNECTION_H
#include <arpa/inet.h>
#include <memory>
#include "socket.h"
#include "../handler/handler.h"
#include "../thread/thread_pool.h"

template<typename Multiplex>
class Connection {
private:
    std::shared_ptr<Socket> _sock;
    std::shared_ptr<Multiplex> _multiplex;
    std::mutex _mtx;

    int _status;
    int _sz;
    int _cur;
    int _flag;
    char _in_buf[1024 * 1024 * 5];

    int _out_cur;
    char _out_buf[1024 * 1024 * 5];

public:
    Connection() {};
    Connection(const std::shared_ptr<Socket>& fd, const std::shared_ptr<Multiplex>& multiplex)
        : _sock(fd), _multiplex(multiplex) {
        _multiplex->add(_sock->fd());
        _status = 0;
        _sz = 0;
        _cur = 0;

        _out_cur = 0;
    }

    ~Connection() {
        _multiplex->rm(_sock->fd());
    }

    int recv() {
        int total = 0;
        while(true) {
            if(_status == 0) {
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
                    //_pool->enqueue(Handler(), type, s);

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

    void callback(const std::string& msg) {
        // printf("callback write msg: %s\n", msg.c_str());

        {
            std::unique_lock<std::mutex> ul(_mtx);
            for(char ch : msg) {
                _out_buf[_out_cur++] = ch;
            }
        }
        //return msg.size();

    }

    // cnt == 0 说明不了神么，就是写不进去数据
    // cnt == -1 接收异常，表明接收出错，断开连接，重置输出缓冲区已经不重要了。
    // cnt == -1 缓冲区已满导致，返回已发送数据
    int send_http() {
        int cnt = 0;
        {
            std::unique_lock<std::mutex> ul(_mtx);
            if(_out_cur) {
                cnt = _sock->sock_send(_out_buf, _out_cur);
            }
            if(cnt == -1) return -1;
            if(cnt != 0) {
                int idx = 0;
                for(int i = cnt; i < _out_cur; ++i) {
                    _out_buf[idx++] = _out_buf[i];
                }
                _out_cur = idx;
            }

        }
        return cnt;
    }

    // total == 0 表明是断开连接请求
    // cnt == -1 且确定是接收异常，则返回 -1，表示接收出错，断开连接
    // cnt == -1 但由于没有数据，则返回总共接收数据。
    int recv_http() {
        int total = 0;
        while(true) {
            int need = sizeof(_in_buf) - _cur;
            int cnt = _sock->sock_recv(_in_buf + _cur, need);
            if(cnt == -1) {
                _reset();
                return -1;
            }
            if(cnt == 0) {
                //_reset();
                return total;
            }
            int pre = std::max(_cur, 3);
            total += cnt;
            _cur += cnt;
            for(int i = pre, pre = 0; i < _cur; ++i) {
                if(_in_buf[i] == '\n' && _in_buf[i - 1] == '\r'
                    && _in_buf[i - 2] == '\n' && _in_buf[i - 3] == '\r') {
                    std::string msg(_in_buf + pre, i + 1 - pre);
                    ThreadPool& pool = ThreadPool::get_instance();
                    pool.enqueue(Handler(), msg, [this](const std::string& s) {
                        this->callback(s);
                    });
                    pre = i + 1;
                }
            }
            if(pre == 0) continue;
            int idx = 0;
            for(int i = pre; i < _cur; ++i, ++idx) {
                _in_buf[idx++] = _in_buf[i];
            }
            _cur = idx;
        }
    }

private:
    void _reset() {
        _cur = 0;
        _status = 0;
        _sz = 0;
    }
};
#endif //WEBSERVER_CONNECTION_H
