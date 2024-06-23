# 项目简介
多线程高并发服务器，允许客户端并发访问服务器上各种 MIME 类型的文件资源

支持 select、poll、epoll 三种 IO 多路复用
- select：最大文件描述符：1024 
- poll：最大文件描述符：2048 
- epoll：最大文件描述符：65535，水平模式 + 边缘模式

支持主从 Reactor，主从 Reactor 任意搭配
- 单个 Main Reactor，多个 Sub Reactor 
- 主 select 从 epoll，主 epoll 从 epoll 等。


# 涉及技术
socket 网络编程、IO 多路复用、多线程、泛型编程

智能指针、C++11 线程、互斥锁

单例模式

mmap

RingBuffer

# 模块

## Socket
RAII 思想：对象创建是申请 sock fd + 自动 “上树”，析构时 close fd + 自动 “下树”？优点：
- 不需要手动管理 fd，自动创建和关闭
- 不需要手动上下树，自动上下树（监听树）

不不不，上述方式会导致在很多情况下使得我们必须加锁来保证程序的正确性，是挺优雅的，但效率上不去...

正确性第一，效率第二，优雅第三，故舍弃 RAII 思想。

### 常用操作
bind、listen、connection等

### accept
以 shared_ptr 形式返回 client_fd？？？？？？

### recv & send
循环接收/发送指定长度的字节数据，返回成功发送/接收字节数
- EINTR 被打断，重新尝试发送/接收
- EAGAIN 读缓冲区已空/写缓冲区已满，不再继续读/写，退出

返回仅有三种状态
- 读/写出错导致返回: 最好处理, 一旦发生直接断开连接即可
- 数据写完返回
- 读缓冲区已空/写缓冲区已满 返回

注意:
- 后两种状态可以根据 已读/写字节数 ?= 要求读/写字节数，来判断是那种返回方式
- 数据读/写完毕，且缓冲区刚好空/满，优先看作数据写完返回


## Server

包含监听套接字及 Main Reactor

启动监听、Main Reactor 线程等

## Main Reactor
包含
- 监听套接字
- 所采用的 IO 多路复用：只监听读事件
- 若干 Sub Reactor：每人单独线程启动

连接读请求到来：
- read_callback accept 接收连接请求
- 将 client_fd 分配给 Sub Reactor：轮询方式（Round-Robin）

## Sub Reactor
包含
- 所采用的 IO 多路复用：读写事件都要监听
- 若干 TcpConnection：
  - 如何维护？map or unordered_map? no for not thread safety!
  - array or vector is better for thread safety

建立新连接顺序（必须遵守，不能颠倒）：
- 1、先构建 connection 对象
- 2、fd 上树

读请求到来时：
- read_callback 读取数据到对应 Connection 输入冲区，若没有读取到数据，说明是断开连接请求
- 将 msg 加入到线程池，交由 worker 处理
- worker 处理完毕，将处理结果写入对应 Connection 输出缓冲区

断开连接顺序（必须遵守，不能颠倒）：
- 1、fd 下树
- 2、析构 connection 对象
- 3、关闭 fd

写请求到来时：
- write_callback 发送对应 Connection 输出缓冲区中数据

## Connection
包含
- client_fd
- 输入缓冲区：仅有 sub reactor thread 访问，无序考虑线程安全问题
- 输出缓冲区：
  - work threads 和 sub reactor thread 都会访问，需要考虑线程安全问题
  - 多生产者，单消费者模型

## Http
Http 请求头解析、Http 响应头封装等

## Html

页面拼接等操作

## Handler

对不同的请求进行处理：
- 文件请求，返回文件数据
- 目录请求，返回当前目录下的文件资源

# Condition Race
## Main Reactor 线程 与 Sub Reactor 线程
Race：
- Main 线程需要新建 Connection
- Sub 线程会移除 Connection


## Worker 线程 与 Sub Reactor 线程

Race：
- Worker 线程执行完毕将数据写入 Connection 输出缓冲区
- 但有可能在之前 Sub Reactor 已经将 Connection 释放掉了


## 输出缓冲区

Race：
- Worker 线程要往 Connection 输出缓冲区中写数据
- Sub Reactor 要从 Connection 输出缓冲区中读数据并发送






