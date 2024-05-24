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

    int _in_cur;
    char _in_buf[1024 * 1024 * 5];

    int _out_cur;
    char _out_buf[1024 * 1024 * 5];

public:
    Connection() {};
    Connection(const std::shared_ptr<Socket>& fd, const std::shared_ptr<Multiplex>& multiplex)
        : _sock(fd), _multiplex(multiplex) {
        //_multiplex->add(_sock->fd());
        _in_cur = 0;

        _out_cur = 0;
    }

    //~Connection() {
    //    _multiplex->rm(_sock->fd());
    //}

    void callback(const std::string& msg) {
        std::unique_lock<std::mutex> ul(_mtx);
        for(char ch : msg) {
            _out_buf[_out_cur++] = ch;
        }
        // 新数据到来，重新激活写事件
	    _multiplex->rm(_sock->fd());
	    _multiplex->add(_sock->fd());
    }

    // cnt == 0 说明不了神么: 可能因为压根就没有数据要写，也可能因为缓冲区一直阻塞写不进去。
    // cnt == -1 接收异常，表明接收出错，断开连接，重置输出缓冲区已经不重要了。
    int send_http() {
        int cnt = 0, status = 0;
        {
            std::unique_lock<std::mutex> ul(_mtx);
            if(_out_cur) {
                cnt = _sock->sock_send(_out_buf, _out_cur, status);
            }
            if(cnt == -1) return -1;
            if(cnt != 0) {
                int idx = 0;
                for(int i = cnt; i < _out_cur; ++i) {
                    _out_buf[idx++] = _out_buf[i];
                }
                _out_cur = idx;
            }
	        // 如果还有数据要写，且以 “缓冲区未被写满” 的状态返回
            // 手动上下树，防止边缘触发 “永久丢失写事件”
            // 没有数据要写了，永久丢失是一件好事情，有新数据到来时会重新激活
	        if(_out_cur != 0 && !status) {
	            _multiplex->rm(_sock->fd());
	            _multiplex->add(_sock->fd());
	        }
        }
        return cnt;
    }

    // total == 0 表明是断开连接请求
    // cnt == -1 表示接收出错，断开连接
    int recv_http() {
        int total = 0;
        while(true) {
            // 一次可能接收不完，先接收一部分，处理了，再接收剩下部分
            int need = sizeof(_in_buf) - _in_cur;
            int cnt = _sock->sock_recv(_in_buf + _in_cur, need);
            if(cnt == -1) {
                return cnt;
            }
            if(cnt == 0) {
                return total;
            }

            int temp = std::max(_in_cur, 3), pre = 0;
            total += cnt; _in_cur += cnt;
            for(int i = temp; i < _in_cur; ++i) {
                if(_in_buf[i] == '\n' && _in_buf[i - 1] == '\r' && _in_buf[i - 2] == '\n' && _in_buf[i - 3] == '\r') {
                    std::string msg(_in_buf + pre, i + 1 - pre);
                    ThreadPool& pool = ThreadPool::get_instance();
                    pool.enqueue(Handler(), msg, [this](const std::string& s) {
                        this->callback(s);
                    });
                    pre = i + 1;
                }
            }
            // 距离上次没有发生移动，消息已经接收完了
            if(pre == 0) {
                return total;
            }
            int idx = 0;
            for(int i = pre; i < _in_cur; ++i, ++idx) {
                _in_buf[idx++] = _in_buf[i];
            }
            _in_cur = idx;
        }
    }
};
#endif //WEBSERVER_CONNECTION_H
