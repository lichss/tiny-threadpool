#include <iostream>
#include "threadpool.h"

void* task(void* arg){
    auto limits = *(int*)arg;
    for (int i=0;i<limits;i++){
        std::cout <<"task1 " << i <<"\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return nullptr;
}

void* task2(void* arg){
    int limits = *(int*)arg;
    for (int i=0;i<limits;i++){
        std::cout <<"task2\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return nullptr;
}

struct args_t
{
    uint32_t limits;
    uint32_t id;
};


int main(){
    std::cout <<"com\n";
    ThreadPool th_pool(4);
    int limits1 = 10;
    int limits2= 20;
    auto thid1 = th_pool.creat_thread(task,&limits1,nullptr,nullptr);
    auto thid2 = th_pool.creat_thread(task2,&limits2,nullptr,nullptr);
    th_pool.detach_thread_(thid1);
    th_pool.join_thread_(thid2);
    
    return 0;
}