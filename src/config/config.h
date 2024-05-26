#ifndef WEBSERVER_CONFIG_H
#define WEBSERVER_CONFIG_H


class Config {
public:
    const static int BUFFER_SIZE = 1024 * 1024 * 5;
    const static int SUB_REACTOR_SIZE = 1;
    const static int LISTEN_QUEUE_SIZE = 10000;
    const static int WORKER_SIZE = 1;
};


#endif //WEBSERVER_CONFIG_H
