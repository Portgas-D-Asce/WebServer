#ifndef WEBSERVER_CONNECTION_H
#define WEBSERVER_CONNECTION_H
#include <arpa/inet.h>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include "../config/config.h"
#include "socket.h"
#include "../buffer/ring_buffer.h"

class Connection {
private:
    std::shared_ptr<Socket> _sock;
    // sock fd can't identify a connection unique
    // use id identify a connection unique
    long long _id;

    RingBuffer _in_buff;
    std::queue<std::tuple<const char* const, size_t, const size_t>> _que;
    mutable std::mutex _mtx;
public:
    explicit Connection(const std::shared_ptr<Socket>& fd) : _sock(fd),
            _in_buff(Config::BUFFER_SIZE) {
        static long long idx = 0;
        _id = idx++;
    }

    long long id() const {
        return _id;
    }

    // sub reactor thread
    ~Connection() {
        // 将未发送响应 mmap 回收
        while(!_que.empty()) {
            auto [ptr, cur, total] = _que.front();
            _que.pop();
            munmap((void*)ptr, total);
        }
    }

    // worker threads
    void callback(const char* ptr, size_t len) {
        std::lock_guard<std::mutex> lg(_mtx);
        // 发送请求加入消息队列
        _que.push({ptr, 0, len});
    }

    // sub reactor thread
    ssize_t send_http() {
        {
            std::lock_guard<std::mutex> lg(_mtx);
            if(_que.empty()) return 0;
        }

        ssize_t total = 0;
        while(true) {
            auto &[ptr, cur, sz] = _que.front();
            // 还剩余多少数据要发送
            ssize_t rem = sz - cur;
            // 发送剩余数据
            ssize_t cnt = _sock->sock_mmap_send(ptr + cur, rem);
            // 接收出错, 直接返回
            if(cnt == -1) return -1;
            total += cnt;

            // 没有写完数据而返回, 则必然是缓冲区满了阻塞了, 更新下次发送起始位置, 直接退出
            cur += cnt;
            if(cnt != rem) break;

            // 写完剩余数据而返回, 任务完成，弹出消息, 释放 mmap 资源, 开始处理下一个消息
            munmap((void*)ptr, sz);
            std::lock_guard<std::mutex> lg(_mtx);
            _que.pop();
            if(_que.empty()) break;
        }
        return total;
    }

    ssize_t recv_http(std::vector<std::string>& msgs) {
        return _in_buff.recv_msg(_sock, "\r\n\r\n", msgs);
    }
};
#endif //WEBSERVER_CONNECTION_H

