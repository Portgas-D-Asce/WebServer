//
// Created by pk on 2024/5/17.
//

#ifndef WEBSERVER_THREAD_POOL_COPY_H
#define WEBSERVER_THREAD_POOL_COPY_H
#include <vector>
#include <iostream>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool {
public:
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    ~ThreadPool();

    static ThreadPool& get_instance() {
        static std::once_flag flag;
        call_once(flag, [&](){
            _instance = std::unique_ptr<ThreadPool>(
                new ThreadPool(4));
        });
        return *_instance;
    }
private:
    explicit ThreadPool(size_t cnt);
private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> _workers;
    // the task queue
    std::queue<std::function<void()>> _tasks;
    // synchronization
    std::mutex _mtx;
    std::condition_variable _cv;
    bool _stop;

    static std::unique_ptr<ThreadPool> _instance;
};

std::unique_ptr<ThreadPool> ThreadPool::_instance = nullptr;

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t cnt) : _stop(false) {
    auto routing = [this]() {
        for (;;) {
            std::function<void()> task;
            printf("thread id: %d\n", std::this_thread::get_id());

            {
                std::unique_lock<std::mutex> lock(this->_mtx);
                // stop == false && task.empty() -> 阻塞
                this->_cv.wait(lock, [this] {
                    return this->_stop || !this->_tasks.empty();
                });

                // stop == true && task.empty() -> 结束线程
                if (this->_stop && this->_tasks.empty()) {
                    return;
                }

                // !task.empty() -> 继续运行
                task = std::move(this->_tasks.front());
                this->_tasks.pop();
            }

            task();
        }
    };

    for(size_t i = 0; i < cnt ; ++i) {
        _workers.emplace_back(routing);
    }
}

// add new work item to the pool
template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(_mtx);

        // don't allow enqueueing after stopping the pool
        if(_stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        _tasks.emplace([task] () {
            (*task)();
        });
    }

    // 为什么这块不需要加锁？？？？？？
    _cv.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _stop = true;
    }
    _cv.notify_all();
    for(std::thread &worker: _workers) {
        worker.join();
    }
}

#endif //WEBSERVER_THREAD_POOL_COPY_H
