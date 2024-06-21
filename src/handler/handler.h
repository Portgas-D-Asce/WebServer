#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H
#include <string>
#include <functional>
#include <sys/mman.h>
#include "file_handler.h"
#include "directory_handler.h"

class Handler {
public:
    int operator()(std::string msg, std::function<void(const char*, size_t)> callback) {
        FileHandler file("../root");
        DirectoryHandler dir("../root");
        std::string temp;
        std::map<std::string, std::string> mp = Http::parse(msg);

        std::string url = mp[Pron[Prop::URL]];
        if(file.check(url)) {
            temp = file.wrapper(url);
        } else if(dir.check(url)) {
            temp = dir.wrapper(url);
        } else {
            temp = file.wrapper("/404.html");
        }

        char* _buff = (char*)mmap(nullptr, temp.size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(_buff == (void*)-1) {
            perror("xxxx");
        }

        memcpy(_buff, temp.c_str(), temp.size());

        // callback 返回消息
        callback(_buff, temp.size());
        return 1;
    }
};

#endif //WEBSERVER_HANDLER_H
