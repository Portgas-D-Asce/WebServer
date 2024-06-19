#ifndef WEBSERVER_INPUT_BUFFER_H
#define WEBSERVER_INPUT_BUFFER_H
#include <string>
#include <vector>

class InputBuffer {
private:
    std::string _buff;
    // _start 起点
    // _end 终点（不包含）
    // _center 检查消息分隔符检查到哪里了
    size_t _start, _center, _end;
public:
    // n 要求必须为 2 的幂
    explicit InputBuffer(size_t n) : _buff(n, '\0'), _start(0), _center(0), _end(0) {}

    // _start == _end 表示没有数据
    bool empty() const {
        return _start == _end;
    }

    // 最多支持 n - 1 个数据, 否则当 _start == _end 分不清缓冲区是空还是满
    // _end + 1 == _start 表示满
    size_t size() const {
        return _end - _start;
    }

    void expand() {
        size_t n = _buff.size(), mask = n - 1;
        // 申请新缓冲区
        std::string temp(n << 1, '\0');
        // 拷贝数据
        for(size_t i = _start, j = 0; i != _end; ++i, ++j) {
            temp[j] = _buff[i & mask];
        }
        // 更新起点, 终点
        _end -= _start;
        _start = 0;
        // 启用新缓冲区
        _buff = std::move(temp);
    }

    // 向缓冲区中写入数据
    void write(const char *s, size_t len) {
        // 总数据 = 当前数据 + 需要写入数据, 如果大于等于缓冲区大小, 则扩容, 直到可以容纳数据
        // 等于时也要扩容, 因为环形缓冲区不能放满
        // 多次扩容肯定不合适, 后续优化
        for(size_t i = 0; size() + len >= _buff.size(); ++i) {
            expand();
        }

        // 将数据写入缓冲区
        size_t n = _buff.size(), mask = n - 1;
        for(size_t i = 0; i < len; ++i, ++_end) {
            _buff[_end & mask] = s[i];
        }
    }

    // 从缓冲区中读取消息
    std::vector<std::string> read(const std::string& split) {
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
            std::string& msg = msgs.back();
            for(size_t i = 0; _start <= _center; ++_start, ++i) {
                msg[i] = _buff[_start & mask];
            }
            _center = _start + m - 1;
        }
        return msgs;
    }
};
#endif //WEBSERVER_INPUT_BUFFER_H
