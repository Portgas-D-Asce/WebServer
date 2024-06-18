#ifndef WEBSERVER_CONFIG_H
#define WEBSERVER_CONFIG_H
#include <string>

class Config {
public:
    const static int BUFFER_SIZE = 1024 * 1024 * 5;
    const static int SUB_REACTOR_SIZE = 1;
    const static int LISTEN_QUEUE_SIZE = 10000;
    const static int WORKER_SIZE = 1;
    const static int EPOLL_MX_EVENT = 1024;
    const static std::string MAIN_REACTOR_NAME;
    const static std::string SUB_REACTOR_NAME;
};

const std::string Config::MAIN_REACTOR_NAME = "main reactor";
const std::string Config::SUB_REACTOR_NAME = "sub reactor";


#endif //WEBSERVER_CONFIG_H
