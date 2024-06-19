#ifndef WEBSERVER_RING_BUFFER_H
#define WEBSERVER_RING_BUFFER_H
#include <string>
#include <vector>
#include <memory>
#include "../common/socket.h"

class RingBuffer {
private:
    std::string _buff;
    // _start 起点
    // _end 终点（不包含）
    // _center 检查消息分隔符检查到哪里了
    size_t _start, _center, _end;
public:
    // n 要求必须为 2 的幂
    explicit RingBuffer(size_t n) : _buff(n, '\0'), _start(0), _center(0), _end(0) {}

    // _start == _end 表示没有数据
    bool empty() const {
        return _start == _end;
    }

    // 最多支持 n - 1 个数据, 否则当 _start == _end 分不清缓冲区是空还是满
    // _end + 1 == _start 表示满
    size_t size() const {
        return _end - _start;
    }

    // 向缓冲区中写入数据
    void write(const char *s, size_t len) {
        // 总数据 = 当前数据 + 需要写入数据, 如果大于等于缓冲区大小, 则扩容, 直到可以容纳数据
        // 等于时也要扩容, 因为环形缓冲区不能放满
        size_t multi = 0;
        while((size() + len) >= (_buff.size() << multi)) ++multi;
        if(multi) _expand(multi);

        // 将数据写入缓冲区
        size_t n = _buff.size(), mask = n - 1;
        for(size_t i = 0; i < len; ++i, ++_end) {
            _buff[_end & mask] = s[i];
        }
    }

    // 从缓冲区中读取消息
    std::vector<std::string> read_msg(const std::string& split) {
        size_t m = split.size(), mask = _buff.size() - 1;
        // 计算消息分隔符 "hash" 值
        unsigned int tar = 0;
        for(size_t i = 0; i < m; ++i) {
            tar = (tar << 8) | split[i];
        }

        // 至少要从 start + m - 1 处开始比较
        _center = std::max(_center, _start + m - 1);
        unsigned int cur = 0;
        for(size_t i = _center + 1 - m; i < _center; ++i) {
            cur = (cur << 8) | _buff[i & mask];
        }

        // 读取消息
        std::vector<std::string> msgs;
        for( ;_center < _end; ++_center) {
            cur = (cur << 8) | _buff[_center & mask];
            if(cur != tar) continue;
            msgs.push_back(std::string(_center - _start + 1, '\0'));
            _copy(msgs.back());
            _start = _center + 1;
            _center = _start + m - 1;
        }
        return msgs;
    }

    int send_msg(const std::shared_ptr<Socket>& sock, int& status) {
        if(empty()) return 0;
        size_t n = _buff.size(), mask = n - 1;
        size_t rel_start = _start & mask, rel_end = _end & mask;

        // 如果数据没有跨越缓冲区末尾, 一次性发送就好了
        if(rel_end > rel_start) {
            int cnt = sock->sock_send(_buff.c_str() + rel_start, rel_end - rel_start, status);
            if(cnt == -1) return -1;
            _start += cnt;
            return cnt;
        }

        // 数据跨越了缓冲区末尾，必须分两部分发送
        int cnt = sock->sock_send(_buff.c_str() + rel_start, n - rel_start, status);
        if(cnt == -1) return -1;

        // 如果以数据没写完方式返回, 那就是缓冲区写满了, 就不要再继续写了
        if(cnt == n - rel_start) {
            int temp = sock->sock_send(_buff.c_str(), rel_end, status);
            if(temp == -1) return -1;
            cnt += temp;
        }
        _start += cnt;

        return cnt;
    }
private:
    void _copy(std::string& t) {
        size_t mask = _buff.size() - 1, rel_start = _start & mask, rel_end = _end & mask;
        if(rel_start <= rel_end) {
            std::copy(_buff.begin() + rel_start, _buff.begin() + rel_end, t.begin());
        } else {
            auto it = std::copy(_buff.begin() + rel_start, _buff.end(), t.begin());
            std::copy(_buff.begin(), _buff.begin() + rel_end, it);
        }
    }

    void _expand(size_t multi) {
        // 申请新缓冲区
        std::string temp(_buff.size() << multi, '\0');
        // 拷贝数据
        _copy(temp);
        // 更新起点, 终点
        _end -= _start;
        _start = 0;
        // 启用新缓冲区
        _buff = std::move(temp);
    }
};
#endif //WEBSERVER_RING_BUFFER_H
