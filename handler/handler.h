//
// Created by pk on 2024/5/20.
//

#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H
#include <string>
#include <functional>
#include "../handler/file_handler.h"
#include "../http/http.h"

class Handler {
public:
    int operator()(std::string msg, std::function<void(const std::string&)> callback) {
        // 解析 input: 先封装 HttpRequest 对象, 然后用其解析消息
        FileHandler handler;
        //printf("%s\n", msg.c_str());

        std::string temp;
        std::map<std::string, std::string> mp = Http::parse(msg);

        std::string url = mp[Pron[Prop::URL]];
        if(!url.empty()) {
            // file_handler or directory_handler
            //printf("file_handler or directory_handler: %s\n", temp.c_str());
            // 包装 output 先封装 HttpResponse 对象, 对返回结果进行封装
            temp = handler.file_wrapper("../root" + url);
        } else if(url == "/dir_handler") {
            // file_handler or directory_handler
            //printf("file_handler or directory_handler: %s\n", temp.c_str());
            // 包装 output 先封装 HttpResponse 对象, 对返回结果进行封装
        } else {
            temp = handler.file_wrapper("../root/404.html");;
        }

        // callback 返回消息
        //printf("callback: \n");
        callback(temp);
        return 1;
    }
};

#endif //WEBSERVER_HANDLER_H
