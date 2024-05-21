//
// Created by pk on 2024/5/20.
//

#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H
#include <string>
#include <functional>
#include "file_handler.h"
#include "directory_handler.h"
#include "../http/http.h"

class Handler {
public:
    int operator()(std::string msg, std::function<void(const std::string&)> callback) {
        // 解析 input: 先封装 HttpRequest 对象, 然后用其解析消息
        FileHandler file("../root");
        DirectoryHandler dir("../root");
        //printf("%s\n", msg.c_str());

        std::string temp;
        std::map<std::string, std::string> mp = Http::parse(msg);

        std::string url = mp[Pron[Prop::URL]];
        if(file.check(url)) {
            //printf("file_handler or directory_handler: %s\n", temp.c_str());
            temp = file.wrapper(url);
        } else if(dir.check(url)) {
            //printf("file_handler or directory_handler: %s\n", temp.c_str());
            temp = dir.wrapper(url);
        } else {
            temp = file.wrapper("/404.html");
        }

        // callback 返回消息
        //printf("callback: \n");
        callback(temp);
        return 1;
    }
};

#endif //WEBSERVER_HANDLER_H
