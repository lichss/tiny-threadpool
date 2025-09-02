#include <iostream>
#include "threadpool.h"
#include <cstdlib>

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

void* getask(void* arg){
    auto args = (args_t*)arg;
    for (int i=0;i<args->limits;i++){
        std::cout <<"getask " << args->id << " : " << i <<"\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return nullptr;
}

int main(){
    std::cout <<"com\n";
    ThreadPool th_pool(4);

    args_t args1{10,1};
    args_t args2{10,2};
    args_t args3{20,3};
    args_t args4{10,4};
    args_t args5{10,5};
    int task_id1 =  
    th_pool.push_task(getask,&args1,nullptr,nullptr);
    int task_id2 =
    th_pool.push_task(getask,&args2,nullptr,nullptr);
    int task_id3 =
    th_pool.push_task(getask,&args3,nullptr,nullptr);

    int task_id4 = th_pool.push_task(getask,&args4,nullptr,nullptr);


    // std::cout <<"task id: " << task_id1 << ", " << task_id2 << ", " << task_id3 << "\n";
    // th_pool.join_thread_id(task_id3);

    if(0 ){/*need help*/
        std::this_thread::sleep_for(std::chrono::seconds(3));
        th_pool.thread_status[0] = THP_IDLE; 
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    int task_id5 = th_pool.push_task(getask,&args5,nullptr,nullptr);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << task_id1 << task_id2 << task_id3 << task_id4 << task_id5;
    // return 0;
    exit(0);

}