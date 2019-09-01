//@bref: 工作线程池，协调进行工作。
//@func: process() ，handler实现process（）方法，工作线程通过调用process进行处理。
//@func: init（）方法初始化线程池相关资源。
//@func：append() 方法实现将handler指针加入线程池。
//

#ifndef MINI_SERVER_THREAD_POOL_H
#define  MINI_SERVER_THREAD_POOL_H

#include <queue>
#include <cstdio>
#include <exception>
#include <new>
#include <stdint.h>
#include <pthread.h>
#include "easylogging++.h"
#include "locker.h"
#include "connection.h"

namespace miniserver {

template <typename T> 
class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    bool append(T *request);
    void run();
    void askToQuit();

    // @func: init 初始化线程池。
    // @param: max_thread 线程池中的工作线程的个数。
    // @param: max_request 请求队列中允许的最大的线程数量。
    bool init(uint32_t max_thread, uint64_t max_requests);

private:
    static void* _thread_handler(void *arg);
    void _destroy();
    
    uint32_t _max_thread;   // 线程池中最大工作线程的数量.
    uint64_t _max_requets;  // 请求队列中允许的最大请求数. 
    pthread_t *_work_threads; // 工作线程存储数组
    std::queue<T*> _request_queue; // 请求队列 
    Locker _queue_lck; // 请求队列锁，解决队列竞争。
    Semaphore _queue_stat; // 信号量来唤醒被线程。
    bool _is_running; // 工作运行标志。
};

template <typename T>
ThreadPool<T>::ThreadPool() :
    _max_thread(0), _max_requets(0),
    _work_threads(nullptr), _request_queue(),
    _queue_lck(), _queue_stat(), _is_running(false) {}

template <typename T> 
ThreadPool<T>::~ThreadPool() {
    _destroy();
}

template <typename T>
void ThreadPool<T>::askToQuit() {
    _is_running = false;
}

template <typename T>
void ThreadPool<T>::_destroy() {
    if (_work_threads != nullptr) {
        delete[] _work_threads;  
    } 
    _is_running = false;
}

template  <typename T>
bool ThreadPool<T>::init(uint32_t max_thread, uint64_t max_requests) {
    if (max_thread < 0 || max_requests < 0) {
        LOG(WARNING) << "[ThreadPllo::init]: invalid parameter"; 
    } 

    _max_thread = max_thread;
    _max_requets = max_requests;

    _work_threads = new(std::nothrow) pthread_t[max_thread];
    if (_work_threads == nullptr) {
        LOG(WARNING) << "[ThreadPool::init]: new _work_thread failed";
        return false;
    }

    LOG(WARNING) << "Create " << _max_thread << " threads in thread pool";
    for (int i = 0; i < _max_thread; ++i) {
        if (pthread_create(_work_threads + i, nullptr, _thread_handler, this) != 0) {
            LOG(WARNING) << "[ThreadPool::init]: pthread_crate failed"; 
            _destroy();
            return false;
        } 
        if (pthread_detach(_work_threads[i] != 0)) {
            LOG(WARNING) << "[ThreadPool::init]: pthread_detach failed"; 
            _destroy();
            return false;
        }
    }
    _is_running = false;
    return true;
}

template <typename T>
bool ThreadPool<T>::append(T *request) {
    if (!_queue_lck.lock()) {
        LOG(WARNING) << "[ThreadPool::append]: lock failed"; 
        return false;
         
    }

    if (_request_queue.size() > _max_requets) {
        if (!_queue_lck.unlock()) {
            LOG(WARNING) << "[ThreadPool::append]: unlock failed in size > _max_requets"; 
            return false;
        }
    }

    _request_queue.push(request);
    if (!_queue_lck.unlock()) {
        LOG(WARNING) << "[ThreadPool::append]: unlock failed"; 
        return false;
    }

    if (!_queue_stat.post()) {
        LOG(WARNING) << "[ThreadPool::append]: semaphore post  failed"; 
        return false;
     
    }

    return true;
}

template <typename T>
void ThreadPool<T>::run() {
    while (_is_running) {
        _queue_stat.wait(); 
        
        if (!_queue_lck.lock()) {
            LOG(WARNING) << "[ThreadPool::run]: _queue_lck lock failed"; 
            return ;
        }

        if (_request_queue.empty()) {
            _queue_lck.unlock();
            continue; 
        }

        T *request = _request_queue.front();
        _request_queue.pop();

        if (!_queue_lck.unlock()) {
            LOG(WARNING) << "[ThreadPool::run]: _queue_lck unlock failed"; 
            return ;
        }

        if (request == nullptr) {
            continue;
        }

        request->process();
    }
}

template <typename T>
void* ThreadPool<T>::_thread_handler(void *arg) {
    if (arg == nullptr) {
        LOG(WARNING) << "[ThreadPool::_thread_handler]: arg  null"; 
        return nullptr;
    }

    ThreadPool *thread_pool = reinterpret_cast<ThreadPool*>(arg);
    if (thread_pool == nullptr) {
        LOG(WARNING) << "[ThreadPool::_thread_handler]: cast  failed"; 
        return nullptr;
    }

    thread_pool->run();
    return thread_pool;
}

}
#endif
