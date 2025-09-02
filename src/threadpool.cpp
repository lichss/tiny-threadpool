#include <thread>
#include <vector>
#include <iostream>
#include "threadpool.h"

ThreadPool::ThreadPool(uint32_t max_thread_n){
    std::lock_guard<std::recursive_mutex> lock(status_mutex);

    MAX_thread_N = max_thread_n;
    thPool.reserve(MAX_thread_N);
    thread_status.resize(MAX_thread_N, 0);
    for(uint32_t i=0; i<MAX_thread_N; i++){
        thPool.emplace_back();
    }

}
ThreadPool::~ThreadPool(){
    std::cout <<"waiting \n";
    for(uint32_t i=0; i<MAX_thread_N; i++){
        if(thPool[i].joinable()){
            thPool[i].join();
        }
    }
    std::cout <<"end \n";
}


bool ThreadPool::empty(){
    return thPool.empty();
}

int32_t ThreadPool::creat_thread(func_ptr f,void* args,func_ptr cb, void* cb_args){
    std::lock_guard<std::recursive_mutex> lock(status_mutex);
    
    for(uint32_t i=0; i<MAX_thread_N;i++){
        if(thread_status[i] == THP_IDLE){
            thread_status[i] = THP_BUSY;
            thPool[i] = std::thread([this,f,args,cb,cb_args,i](){
                f(args);
                if(cb != nullptr){
                    cb(cb_args);
                }
                this->thread_status[i] = THP_IDLE;
            });
            return i;
        }
    }
    return -1;
}

uint32_t ThreadPool::create_detach_thread(func_ptr f,void* args){
    std::lock_guard<std::recursive_mutex> lock(status_mutex);
    for(uint32_t i=0; i<MAX_thread_N;i++){
        if(thread_status[i] == 0){
            thread_status[i] = 1;
            thPool[i] = std::thread(f, args);
            thPool[i].detach();
            return i;
        }
    }
    return -1;
}

pool_thread_id ThreadPool::detach_thread_id(pool_thread_id id){
    if(id >= MAX_thread_N)
        return -1;
    std::lock_guard<std::recursive_mutex> lock(status_mutex);
    if(thread_status[id] == THP_IDLE)
        return -1;
    if(thread_status[id] == THP_BUSY){
        if(thPool[id].joinable()){
            thPool[id].detach();
            return id;
        }
    }
    
    return -1;
}

uint32_t ThreadPool::create_join_thread(func_ptr f,void* args){
    std::lock_guard<std::recursive_mutex> lock(status_mutex);
    for(uint32_t i=0; i<MAX_thread_N;i++){
        if(thread_status[i] == 0){
            thread_status[i] = 1;
            thPool[i] = std::thread(f, args);
            return i;
        }
    }
    return -1;
}



int32_t ThreadPool::join_thread_id(uint32_t id){
    if(id >= MAX_thread_N)
        return -1;

    // 先判断状态并取出 joinable 状态
    bool need_join = false;
    {
        std::lock_guard<std::recursive_mutex> lock(status_mutex);
        if(thread_status[id] == THP_IDLE)
            return -1;
        if(thPool[id].joinable())
            need_join = true;
        else
            return -1;
    }

    // 在锁外 join，避免阻塞其他线程
    if(need_join)
        thPool[id].join();

    // join 后再更新状态
    {
        std::lock_guard<std::recursive_mutex> lock(status_mutex);
        thread_status[id] = THP_IDLE;
    }
    return id;
}

int32_t ThreadPool::push_task(func_ptr task,void* args, func_ptr cb, void* cb_args){
    if(task == nullptr)
        return -1;

    std::lock_guard<std::recursive_mutex> lock(status_mutex);
    std::vector<func_list> task_list;
    task_list.push_back({(void*)task, args, (void*)cb, cb_args});

    task_queue.push(task_list);
    int task_id = 
    arrange_task();


    return task_id;
}

int32_t ThreadPool::start_thread(func_ptr f,void* args,func_ptr cb, void* cb_args){
    if(f == nullptr)
        return -1;
    std::lock_guard<std::recursive_mutex> lock(status_mutex);
    for(uint32_t i=0; i<MAX_thread_N;i++){
        if(thread_status[i] == THP_IDLE){
            if(task_queue.empty())
                break;
            auto task_list = task_queue.front();
            task_queue.pop();
            thread_status[i] = THP_BUSY;
            thPool[i] = std::thread([this,task_list,i](){
                for(auto f_list : task_list){
                    func_ptr f = (func_ptr)f_list[0];
                    void* args = f_list[1];
                    func_ptr cb = (func_ptr)f_list[2];
                    void* cb_args = f_list[3];
                    f(args);
                    if(cb != nullptr){
                        cb(cb_args);
                    }
                }
                this->thread_status[i] = THP_IDLE;
                return i;
            });
            // pool[i].detach();
        }
    }
    return -1; 
}

// int32_t ThreadPool::arrange_task(){
//     std::lock_guard<std::recursive_mutex> lock(status_mutex);
//     if(task_queue.empty())
//         return -1;
//     for(uint32_t i=0; i<MAX_thread_N;i++){
//         if(thread_status[i] == THP_IDLE){
//             if(task_queue.empty())
//                 break;
//             auto task_list = task_queue.front();
//             task_queue.pop();
//             thread_status[i] = THP_BUSY;
//             thPool[i] = std::thread([this,task_list,i](){
//                 for(auto f_list : task_list){
//                     func_ptr f = (func_ptr)f_list[0];
//                     void* args = f_list[1];
//                     func_ptr cb = (func_ptr)f_list[2];
//                     void* cb_args = f_list[3];
//                     f(args);
//                     if(cb != nullptr){
//                         cb(cb_args);
//                     }
//                 }
//                 this->thread_status[i] = THP_IDLE;
//             });
//             return i;

//             // pool[i].detach();
//         }
//     }
//     return -1;
// }

int32_t ThreadPool::arrange_task(){
    std::lock_guard<std::recursive_mutex> lock(status_mutex);
    if(task_queue.empty())
        return -1;
    for(uint32_t i=0; i<MAX_thread_N; i++){
        if(thread_status[i] == THP_IDLE){
            // 如果线程对象已经结束，先回收
            if(thPool[i].joinable()) {
                thPool[i].join();
            }
            // 重新创建线程对象
            if(task_queue.empty())
                break;
            auto task_list = task_queue.front();
            task_queue.pop();
            thread_status[i] = THP_BUSY;
            thPool[i] = std::thread([this,task_list,i](){
                for(auto f_list : task_list){
                    func_ptr f = (func_ptr)f_list[0];
                    void* args = f_list[1];
                    func_ptr cb = (func_ptr)f_list[2];
                    void* cb_args = f_list[3];
                    f(args);
                    if(cb != nullptr){
                        cb(cb_args);
                    }
                }
                this->thread_status[i] = THP_IDLE;
            });
            return i;
        }
        // 如果线程对象不是空闲但已经结束，也可以重建
        else if(!thPool[i].joinable()) {
            // 重新创建线程对象
            if(task_queue.empty())
                break;
            auto task_list = task_queue.front();
            task_queue.pop();
            thread_status[i] = THP_BUSY;
            thPool[i] = std::thread([this,task_list,i](){
                for(auto f_list : task_list){
                    func_ptr f = (func_ptr)f_list[0];
                    void* args = f_list[1];
                    func_ptr cb = (func_ptr)f_list[2];
                    void* cb_args = f_list[3];
                    f(args);
                    if(cb != nullptr){
                        cb(cb_args);
                    }
                }
                this->thread_status[i] = THP_IDLE;
            });
            return i;
        }
    }
    return -1;
}

void ThreadPool::reset_thread_to_idle(uint32_t id) {
    if (id >= MAX_thread_N) return;
    std::lock_guard<std::recursive_mutex> lock(status_mutex);
    thread_status[id] = THP_IDLE; // 0 表示空闲
    // if (thread_status[id] == 2) { // 2 表示完成
    //     thread_status[id] = THP_IDLE; // 0 表示空闲
    // }
}

/**
 * 下面的程序仅用于临时测试
 */
#if 0 
void* task(void* arg){
    for (;true;){
        std::cout <<"task1\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void* task2(void* arg){
    for (;true;){
        std::cout <<"task2\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(){
    std::cout <<"com\n";
    ThreadPool th_pool(4);
    th_pool.push_thread(task,nullptr);
    th_pool.push_thread(task2,nullptr);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
#endif