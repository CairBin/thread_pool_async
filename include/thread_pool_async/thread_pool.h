#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <map>
#include <future>
#include <memory>

namespace thread_pool_async{

class ThreadPool{
public:
    // max thread number is equal CPU core number in default condition
    ThreadPool(uint32_t min = 2,
               uint32_t max = 
               std::
               thread::
               hardware_concurrency());

    ~ThreadPool();
    
    void AddTask(std::function<void(void)> task);
    
    template<typename F, typename... Args>
    auto AddTask(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>{
        
        using ReturnType = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<ReturnType()> > (
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ReturnType> res = task->get_future();
        
        {
            std::lock_guard<std::mutex> locker(queue_mutex_);
            tasks_.emplace([task](){
                (*task)();
            });
        }

        condition_.notify_one();

        return res;
    }

private:
    void Manager();
    void Worker();
private:
    std::thread* manager_;
    std::vector<std::thread::id> ids_; // the thread that has exited id
    std::map<std::thread::id, std::thread> workers_;
    std::atomic<int> min_threads_number_;
    std::atomic<int> max_threads_number_;
    std::atomic<int> current_thread_;
    std::atomic<int> idle_thread_;
    std::atomic<int> exit_thread_;
    std::atomic<bool> is_stop_; //switch

    std::queue<std::function<void(void)> > tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::mutex ids_mutex_;
};

}

#endif
