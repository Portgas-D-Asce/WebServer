#ifndef WEBSERVER_THREAD_POOL_H
#define WEBSERVER_THREAD_POOL_H
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
#include "../config/config.h"

class ThreadPool {
public:
    static ThreadPool& get_instance();

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    friend class std::default_delete<ThreadPool>;
    explicit ThreadPool(size_t cnt);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ~ThreadPool();

    void routing();
private:
    // need to keep track of threads, so we can join them
    std::vector<std::thread> _workers;
    // the task queue
    std::queue<std::function<void()>> _tasks;
    // synchronization
    mutable std::mutex _mtx;
    mutable std::condition_variable _cv;
    bool _stop;

    //singleton
    static std::unique_ptr<ThreadPool> _instance;
};

std::unique_ptr<ThreadPool> ThreadPool::_instance = nullptr;

inline ThreadPool& ThreadPool::get_instance() {
    static std::once_flag flag;
    call_once(flag, [&](){
        _instance = std::unique_ptr<ThreadPool>(
            new ThreadPool(Config::WORKER_SIZE));
    });
    return *_instance;
}

void ThreadPool::routing() {
    std::function<void()> task;
    for(;;) {
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
}

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t cnt) : _stop(false) {
    for(size_t i = 0; i < cnt ; ++i) {
        _workers.emplace_back(std::bind(&ThreadPool::routing, this));
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

#endif //WEBSERVER_THREAD_POOL_H
