//
// Created by pk on 2024/5/20.
//

#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H
#include <string>
#include <functional>
#include "../http/http_request.h"
#include "../http/http_response.h"

class Handler {
public:
    int operator()(std::string msg, std::function<void(const std::string&)> callback) {
        // 解析 input: 先封装 HttpRequest 对象, 然后用其解析消息
        HttpRequest req;
        HttpResponse resp;
        printf("%s\n", msg.c_str());

        if(!req.parse(msg)) {
            callback(resp.not_found());
            return -1;
        }

        std::string url = req.url();

        std::string temp = "test";
        if(url == "file_handler") {
            // file_handler or directory_handler
            printf("file_handler or directory_handler: %s\n", temp.c_str());
            // 包装 output 先封装 HttpResponse 对象, 对返回结果进行封装

        } else if(url == "dir_handler") {
            // file_handler or directory_handler
            printf("file_handler or directory_handler: %s\n", temp.c_str());
            // 包装 output 先封装 HttpResponse 对象, 对返回结果进行封装
        } else {
            temp = resp.not_found();
        }

        // callback 返回消息
        printf("callback: \n");
        callback(temp);
        return 1;
    }
};


#endif //WEBSERVER_HANDLER_H
