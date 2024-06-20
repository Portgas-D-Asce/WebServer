#ifndef WEBSERVER_CONNECTION_H
#define WEBSERVER_CONNECTION_H
#include <arpa/inet.h>
#include <memory>
#include <vector>
#include <mutex>
#include "../config/config.h"
#include "socket.h"
#include "../buffer/ring_buffer.h"

template<typename Multiplex>
class Connection {
private:
    std::shared_ptr<Socket> _sock;
    //std::shared_ptr<Multiplex> _multiplex;
    mutable std::mutex _mtx;
    // sock fd can't identify a connection unique
    // use id identify a connection unique
    long long _id;

    RingBuffer _in_buff;
    RingBuffer _out_buff;

public:
    Connection() {};
    Connection(const std::shared_ptr<Socket>& fd/*, const std::shared_ptr<Multiplex>& multiplex*/)
        : _sock(fd)/*, _multiplex(multiplex)*/, _in_buff(Config::BUFFER_SIZE), _out_buff(Config::BUFFER_SIZE) {
        static long long idx = 0;
        _id = idx++;
        //_multiplex->add(_sock->fd());
    }

    long long id() const {
        return _id;
    }

    //~Connection() {
    //    _multiplex->rm(_sock->fd());
    //}

    void callback(const std::string& msg) {
        std::lock_guard<std::mutex> lg(_mtx);
        _out_buff.write(msg.c_str(), msg.size());
        // 新数据到来，重新激活写事件
	    // _multiplex->mod(_sock->fd());
    }

    // cnt == 0 说明不了神么: 可能因为压根就没有数据要写，也可能因为缓冲区一直阻塞写不进去。
    // cnt == -1 接收异常，表明接收出错，断开连接，重置输出缓冲区已经不重要了。
    int send_http() {
        // int cnt = 0, status = 0;
        // {
        //     std::lock_guard<std::mutex> ul(_mtx);
        //     if(!_out_buff.empty()) {
        //         cnt = _out_buff.send_msg(_sock, status);
        //         if(cnt == -1) return -1;
        //     }

        //     // 如果还有数据要写，且以 “缓冲区未被写满” 的状态返回(never happen!)
        //     // 手动上下树，防止边缘触发 “永久丢失写事件”
        //     // 没有数据要写了，永久丢失是一件好事情，有新数据到来时会重新激活
        //     // if(!_out_buff.empty() && !status) {
        //     //     _multiplex->mod(_sock->fd());
        //     // }
        // }
        // return cnt;

        int cnt = 0;
        {
            std::lock_guard<std::mutex> ul(_mtx);
            if(!_out_buff.empty()) {
                cnt = _out_buff.send_msg(_sock);
                if(cnt == -1) return -1;
            }

            // 如果还有数据要写，且以 “缓冲区未被写满” 的状态返回(never happen!)
            // 手动上下树，防止边缘触发 “永久丢失写事件”
            // 没有数据要写了，永久丢失是一件好事情，有新数据到来时会重新激活
            // if(!_out_buff.empty() && !status) {
            //     _multiplex->mod(_sock->fd());
            // }
        }
        return cnt;
    }

    // total == 0 表明是断开连接请求
    // cnt == -1 表示接收出错，断开连接
    // int recv_http(std::vector<std::string>& msgs) {
    //     int total = 0, status = 0;
    //     char buff[4096] = {0};
    //     while(true) {
    //         int cnt = _sock->sock_recv(buff, sizeof(buff), status);
    //         if(cnt == -1) {
    //             return cnt;
    //         }
    //         if(cnt == 0) {
    //             break;
    //         }
    //         total += cnt;
    //         _in_buff.write(buff, cnt);
    //     }
    //     msgs = _in_buff.read_msg("\r\n\r\n");
    //     return total;
    // }

    int recv_http(std::vector<std::string>& msgs) {
        int total = 0, status = 0;
        char buff[4096] = {0};
        while(true) {
            int cnt = _sock->sock_recv(buff, sizeof(buff));
            if(cnt == -1) {
                return cnt;
            }
            if(cnt == 0) {
                break;
            }
            total += cnt;
            _in_buff.write(buff, cnt);
        }
        msgs = _in_buff.read_msg("\r\n\r\n");
        return total;
    }
};
#endif //WEBSERVER_CONNECTION_H

