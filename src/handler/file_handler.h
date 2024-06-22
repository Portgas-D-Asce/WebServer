#ifndef WEBSERVER_FILE_HANDLER_H
#define WEBSERVER_FILE_HANDLER_H
#include <string>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstring>
#include "../http/http.h"

class FileHandler {
private:
    std::string _root;
public:
    explicit FileHandler(std::string root) : _root(root) {}

    std::pair<char*, size_t> wrapper(const std::string& path) const {
        int fd = open((_root + path).c_str(), O_RDWR);
        if(fd == -1) perror("open failed");
        // 获取文件大小
        size_t len = lseek(fd, 0, SEEK_END);
        char* content = (char*)mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        close(fd);

        // 封装响应头
        std::string suf = path.substr(path.find_last_of('.') + 1);
        std::string header = Http::get_header(Stat::OK, suf, len);

        size_t total = header.size() + len;
        char* ptr = (char*)mmap(nullptr, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        // 拷贝响应头数据
        memcpy(ptr, header.c_str(), header.size());
        // 拷贝文件数据
        memcpy(ptr + header.size(), content, len);
        munmap(content, len);

        return {ptr, total};
    }

    bool check(const std::string& url) const {
        struct stat info;
        std::string path = _root + url;
        if(stat(path.c_str(), &info) == -1) {
            return false;
        }
        return info.st_mode & S_IFREG;
    }
};


#endif //WEBSERVER_FILE_HANDLER_H
