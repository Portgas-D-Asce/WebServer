# 项目简介
主从 Reactor，多线程高并发服务器。客户端并发访问服务器上各种 MIME 类型
的文件资源

## 涉及技术
socket 网络编程、IO 多路复用、多线程、泛型编程

智能指针、C++11 线程、互斥锁

单例模式

## IO 多路复用
支持 select、poll、epoll 三种 IO 多路复用

### select
最大文件描述符：1024

### poll
最大文件描述符：2048

### epoll
最大文件描述符：65535

水平模式 + 边缘模式

## Reactor
主从 Reactor：单个 Main Reactor，多个 Sub Reactor

主从 Reactor 任意搭配：主 select 从 epoll，主 epoll 从 epoll 等。

# 模块

## Socket
RAII 思想：对象创建是申请 sock fd，析构时 close fd

### 常用操作
bind、listen、connection等

### accept
以 shared_ptr 形式返回 client_fd，接受请求则妥善保存，否则自动释放

### recv & send
循环接收/发送指定长度的字节数据，返回成功发送/接收字节数、状态

EINTR 重新尝试发送/接收

EAGAIN 退出，返回成功发送/接收到的字节数

## Server

包含监听套接字及 Main Reactor

启动监听、Main Reactor 线程等

## Main Reactor
包含监听套接字、Main Reactor 所采用的 IO 多路复用、若干 Sub Reactor

连接读请求到来时：
- read_callback 接收连接请求
- 将 client_fd 分配给 Sub Reactor

断开连接请求到来时：
- 通知 Sub Reactor 释放 TcpConnection 资源，并从 IO 多路复用中移除监听


## Sub Reactor
包含 Sub Reactor 所采用的 IO 多路复用、若干 TcpConnection

读请求到来时：
- read_callback 读取数据到对应 TcpConnection 输入冲区
- 并将 msg 加入到线程池，交由 worker 处理
- worker 处理完毕，将处理结果写入对应 TcpConnection 输出缓冲区

写请求到来时：
- write_callback 发送对应 TcpConnection 输出缓冲区中数据

## TcpConnection
包含 client_fd、输入缓冲区、输出缓冲区、在哪个 IO 多路复用上

## Http

负责 Http 请求头解析、Http 请求头封装等

## Html

负责页面拼接等操作

## Handler

对不同的请求进行处理：
- 文件请求，返回文件数据
- 目录请求，返回当前目录下的文件资源

# Condition Race
## Main Reactor 线程 与 Sub Reactor 线程
Race：
- Main 线程需要新建 TcpConnection
- Sub 线程会移除 TcpConnection


## Worker 线程 与 Sub Reactor 线程

Race：
- Worker 线程执行完毕将数据写入 TcpConnection 输出缓冲区
- 但有可能在之前 Sub Reactor 已经将 TcpConnection 释放掉了


## 输出缓冲区

Race：
- Worker 线程要往 TcpConnection 输出缓冲区中写数据
- Sub Reactor 要从 TcpConnection 输出缓冲区中读数据并发送






