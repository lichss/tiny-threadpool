#include <thread>
#include <ctype.h>
#include <mutex>
#include <vector>

#define THP_BUSY    1u
#define THP_IDLE    0u
typedef void* (*func_ptr)(void*);
typedef uint32_t pool_thread_id;
class ThreadPool{
public:
    std::mutex status_mutex;
    ThreadPool(uint32_t mat_thread_n);
    ~ThreadPool();
    ThreadPool& operator=(const ThreadPool&) = delete; /*禁用拷贝赋值。这对于整个线程池库非常重要 */
    uint32_t creat_thread(func_ptr f,void* args,func_ptr cb, void* cb_args);
    uint32_t detach_thread(func_ptr f,void* args);
    pool_thread_id detach_thread_(pool_thread_id id);
    uint32_t join_thread(func_ptr f,void* args);
    uint32_t join_thread_(pool_thread_id id);
    bool empty();
    bool use_thread(void* f);
    uint32_t MAX_thread_N;
    std::vector<std::thread> pool;
private:
    uint32_t current_thread_n = 0;
    std::vector<uint32_t> thread_status; // 0-idle, 1-busy, 2-finish

};