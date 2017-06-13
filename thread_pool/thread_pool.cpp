// Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
// @brief 

#include <unistd.h>
#include <com_log.h>
#include "thread_pool.h"

namespace anti {
namespace themis {
namespace common_lib {

DEFINE_int32(thread_pool_capacity, 1, "default thread num");

static const int32_t MS = 1000;

ThreadPool::ThreadPool(int32_t backend_threadnum) {
    _init(backend_threadnum);    
}
ThreadPool::~ThreadPool() {
    exit();    
}
ThreadPool* ThreadPool::instance() {
    static ThreadPool pool(FLAGS_thread_pool_capacity);
    return &pool;
}

bool ThreadPool::_init(int32_t backend_threadnum) {
    for (int32_t i = 0; i < backend_threadnum; ++i) {
        _threads.emplace_back(
                make_unique<std::thread>([&] {
                    backend_run(&_task_queue); 
                }));
    }

    _write_thread = std::move(
            make_unique<std::thread>([&] {
                backend_run(&_data_out_queue);
            }));
    return true;
}

void ThreadPool::exit() {
    _task_queue.exit();
    _data_out_queue.exit();
    for (auto& obj : _threads) {
        obj->join();
    }
    if (_write_thread) {
        _write_thread->join();
        _write_thread.reset(NULL);
    }
    _threads.clear();
}

void ThreadPool::backend_run(TaskQueue* queue) {
    while (!queue->is_exit()) {
        auto obj = std::move(queue->consume());
        if (obj) { obj->run(); }
        else { usleep(50 * MS); }
    }
}

}
}  // namespace themis
}  // namespace anti

// Codes are auto generated by God
