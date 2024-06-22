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
        std::map<std::string, std::string> mp = Http::parse(msg);

        std::string url = mp[Pron[Prop::URL]];
        if(dir.check(url)) {
            auto [ptr, len] = dir.wrapper(url);
            // callback 返回消息
            callback(ptr, len);
        } else {
            if(!file.check(url)) url = "/404.html";
            auto [ptr, len] = file.wrapper(url);
            callback(ptr, len);
        }
        return 1;
    }
};

#endif //WEBSERVER_HANDLER_H
