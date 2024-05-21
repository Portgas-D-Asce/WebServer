#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H
#include <string>
#include <functional>
#include "file_handler.h"
#include "directory_handler.h"

class Handler {
public:
    int operator()(std::string msg, std::function<void(const std::string&)> callback) {
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

        // callback 返回消息
        callback(temp);
        return 1;
    }
};

#endif //WEBSERVER_HANDLER_H
