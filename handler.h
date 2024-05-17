//
// Created by pk on 2024/5/17.
//

#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H
#include <iostream>
#include <string>
#include <thread>

class Handler {
public:
    void operator()(int type, const std::string& s) {
        printf("thread %d: %d, %s\n", std::this_thread::get_id(), type, s.c_str());
    }
};


#endif //WEBSERVER_HANDLER_H
