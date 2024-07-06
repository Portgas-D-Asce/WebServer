#ifndef WEBSERVER_MMAP_RING_BUFFER_H
#define WEBSERVER_MMAP_RING_BUFFER_H
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <sys/mman.h>
#include "../common/socket.h"

class RingBuffer {
private:
    char* _buff;
    size_t _n;
    // _start 起点
    // _end 终点（不包含）
    size_t _start, _end;
public:
    // n 要求必须为 2 的幂
    explicit RingBuffer(size_t n) : _n(n), _start(0), _end(0) {
        _buff = (char*)mmap(nullptr, _n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }

    ~RingBuffer() {
        munmap(_buff, _n);
    }

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
        return _n;
    }

    void clear() {
        _start = _end;
    }

    ssize_t recv_msg(const std::shared_ptr<Socket>& sock, const std::string& split, std::vector<std::string>& msgs) {
        ssize_t total = 0;
        // 数据读完之后从 center 处开始查找消息分隔符号
        size_t center = _end - _start;

        while(true) {
            if(_end - _start == _n) _expand();
            size_t mask = _n - 1, real_start = _start & mask, real_end = _end & mask;
            // 剩余部分在中间
            if(real_start > real_end) {
                size_t need = real_start - real_end;
                ssize_t cnt = sock->sock_recv(_buff + real_end, need);
                if(cnt == -1) return cnt; if(cnt == 0) break;
                total += cnt; _end += cnt;
            } else {
                size_t part_right = _n - real_end;
                ssize_t cnt = 0;
                if(part_right) {
                    cnt = sock->sock_recv(_buff + real_end, part_right);
                    if(cnt == -1) return cnt; if(cnt == 0) break;
                    total += cnt; _end += cnt;
                }

                // 不检查 real_start 会导致，缓冲区已满，但数据未接收完，导致返回
                if(cnt == part_right && real_start) {
                    cnt = sock->sock_recv(_buff, real_start);
                    if(cnt == -1) return cnt; if(cnt == 0) break;
                    total += cnt; _end += cnt;
                }
            }
        }
        msgs = _read_msg(split, _start + center);
        return total;
    }
private:
    void _copy_from_ring(char* t) {
        size_t mask = _n - 1, real_start = _start & mask, real_end = _end & mask;
        // 数据位于中间部分（缓冲区满了也无法使用该拷贝方式）
        if(real_start <= real_end && _end - _start != _n) {
            memcpy( t, _buff + real_start, real_end - real_start);
        } else {
            memcpy(t, _buff + real_start, _n - real_start);
            memcpy(t + _n - real_start, _buff, real_end);
        }
    }

    void _expand() {
        // 申请新缓冲区
        char* temp = (char*)mmap(nullptr, _n << 1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        // 拷贝数据
        _copy_from_ring(temp);
        munmap(_buff, _n);
        _n <<= 1;
        // 更新起点, 终点
        _end -= _start;
        _start = 0;
        // 启用新缓冲区
        _buff = temp;
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
        for( ;center < _end; ++center) {
            cur = (cur << 8) | _buff[center & mask];
            if(cur != tar) continue;
            msgs.push_back(std::string(center - _start + 1, '\0'));
            _copy_from_ring(const_cast<char*>(msgs.back().c_str()));
            _start = center + 1;
            center = _start + m - 1;
        }
        return msgs;
    }
};


#endif //WEBSERVER_MMAP_RING_BUFFER_H
