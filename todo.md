select clientfd >= 1024 情况记得出列

epoll clientfd >= 数组长度时记得处理

非阻塞模式概率性失败

这破烂缓冲区也能用。。。抽空得搞下

多个从 reactor 时，tcpconnection 如何分配？