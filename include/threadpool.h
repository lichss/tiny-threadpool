#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
// #include <ctype.h>
#include <cctype>
#include <mutex>
#include <vector>
#include <queue>


#define THP_BUSY    1u
#define THP_IDLE    0u
typedef void* (*func_ptr)(void*);
typedef std::vector<void*> func_list;


typedef uint32_t pool_thread_id;
class ThreadPool{
public:
    std::recursive_mutex status_mutex; /* sorry 我知道可重入锁不好 我可能会在后续改进*/
    ThreadPool(uint32_t mat_thread_n);
    ~ThreadPool();
    ThreadPool& operator=(const ThreadPool&) = delete; /*禁用拷贝赋值。这对于整个线程池库非常重要 */
    int32_t creat_thread(func_ptr f,void* args,func_ptr cb, void* cb_args);
    uint32_t create_detach_thread(func_ptr f,void* args); /*等着删*/
    pool_thread_id detach_thread_id(pool_thread_id id);
    uint32_t create_join_thread(func_ptr f,void* args); /*等着删 */
    int32_t join_thread_id(pool_thread_id id); 
    int32_t start_thread(func_ptr f,void* args,func_ptr cb, void* cb_args);
    bool empty();
    uint32_t MAX_thread_N;
    std::vector<std::thread> pool;
    std::queue<std::vector< func_list>> task_queue;
    int32_t push_task(func_ptr task,void* args, func_ptr cb, void* cb_args);
    int32_t arrange_task(); 
private:
    uint32_t current_thread_n = 0;
    std::vector<uint32_t> thread_status; // 0-idle, 1-busy, 2-finish


};

#endif // THREADPOOL_H