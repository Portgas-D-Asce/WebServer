#ifndef WEBSERVER_RING_BUFFER_H
#define WEBSERVER_RING_BUFFER_H
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include "../common/socket.h"

class RingBuffer {
private:
    std::string _buff;
    // _start 起点
    // _end 终点（不包含）
    size_t _start, _end;
    mutable std::mutex _mtx;
public:
    // n 要求必须为 2 的幂
    explicit RingBuffer(size_t n) : _buff(n, '\0'), _start(0), _end(0) {}

    // _start == _end 表示没有数据
    bool empty() const {
        return _start == _end;
    }

    // 最多支持 n - 1 个数据, 否则当 _start == _end 分不清缓冲区是空还是满
    // _end + 1 == _start 表示满
    size_t size() const {
        return _end - _start;
    }

    size_t capacity() const {
        return _buff.size();
    }

    size_t clear() {
        _start = _end;
    }

    // 向缓冲区中写入数据
    void write_msg(const char* msg, size_t m, bool flag = true) {
        // 总数据 = 当前数据 + 需要写入数据, 如果大于等于缓冲区大小, 则扩容, 直到可以容纳数据
        // 等于时也要扩容, 因为环形缓冲区不能放满
        size_t multi = 0, total = size() + m;
        while(total >= (_buff.size() << multi)) ++multi;
        if(flag) std::lock_guard<std::mutex> lg(_mtx);
        if(multi) _expand(multi);

        // 将数据写入缓冲区
        _copy_to_ring(msg, m);
    }

    int recv_msg(const std::shared_ptr<Socket>& sock, const std::string& split, std::vector<std::string>& msgs) {
        int total = 0;
        char buff[8192] = {0};
        // 数据读完之后从 center 处开始查找消息分隔符号
        size_t center = end();
        while(true) {
            int cnt = sock->sock_recv(buff, sizeof(buff));
            if(cnt == -1) {
                return cnt;
            }
            if(cnt == 0) {
                break;
            }
            total += cnt;
            write_msg(buff, cnt, false);
        }
        msgs = _read_msg(split, center);
        return total;
    }

    int send_msg(const std::shared_ptr<Socket>& sock) {
        std::lock_guard<std::mutex> lg(_mtx);
        if(empty()) return 0;
        size_t n = capacity(), rel_start = real_start(), rel_end = real_end();
        // 如果数据没有跨越缓冲区末尾, 一次性发送就好了
        if(rel_end > rel_start) {
            int cnt = sock->sock_send(_buff.c_str() + rel_start, rel_end - rel_start);
            if(cnt == -1) return -1;
            _start += cnt;
            return cnt;
        }
        // 数据跨越了缓冲区末尾，必须分两部分发送
        int cnt = sock->sock_send(_buff.c_str() + rel_start, n - rel_start);
        if(cnt == -1) return -1;
        // 如果以数据没写完方式返回, 那就是缓冲区写满了, 就不要再继续写了
        if(cnt == n - rel_start) {
            int temp = sock->sock_send(_buff.c_str(), rel_end);
            if(temp == -1) return -1;
            cnt += temp;
        }
        _start += cnt;
        return cnt;
    }

private:
    size_t start() const {
        return _start;
    }

    size_t end() const {
        return _end;
    }

    size_t real_start() const {
        return _start & (capacity() - 1);
    }

    size_t real_end() const {
        return _end & (capacity() - 1);
    }

    void _copy_to_ring(const char* msg, size_t m) {
        size_t n = _buff.size(), rel_end = _end & (n - 1);
        if(n - rel_end >= m) {
            std::copy(msg, msg + m, _buff.begin() + rel_end);
        } else {
            size_t temp = n - rel_end;
            std::copy(msg, msg + temp, _buff.begin() + rel_end);
            std::copy(msg + temp, msg + m, _buff.begin());
        }
        _end += m;
    }

    void _copy_from_ring(std::string& t) {
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
        _copy_from_ring(temp);
        // 更新起点, 终点
        _end -= _start;
        _start = 0;
        // 启用新缓冲区
        _buff = std::move(temp);
    }

    // 从缓冲区中读取消息
    std::vector<std::string> _read_msg(const std::string& split, size_t center) {
        if(empty()) return {};

        size_t m = split.size(), mask = capacity() - 1;
        // 计算消息分隔符 "hash" 值
        unsigned int tar = 0;
        for(size_t i = 0; i < m; ++i) {
            tar = (tar << 8) | split[i];
        }

        // 至少要从 start + m - 1 处开始比较
        center = std::max(center, _start + m - 1);
        unsigned int cur = 0;
        for(size_t i = center + 1 - m; i < center; ++i) {
            cur = (cur << 8) | _buff[i & mask];
        }

        // 读取消息
        std::vector<std::string> msgs;
        msgs.reserve(10);
        for( ;center < _end; ++center) {
            cur = (cur << 8) | _buff[center & mask];
            if(cur != tar) continue;
            msgs.push_back(std::string(center - _start + 1, '\0'));
            _copy_from_ring(msgs.back());
            _start = center + 1;
            center = _start + m - 1;
        }
        return msgs;
    }
};
#endif //WEBSERVER_RING_BUFFER_H
