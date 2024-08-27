#include "thread_pool_async/thread_pool.h"

namespace thread_pool_async{

ThreadPool::ThreadPool(uint32_t min, uint32_t max)
    : min_threads_number_(min),
      max_threads_number_(max),
      is_stop_(false),
      exit_thread_(0){

    idle_thread_ = min;
    current_thread_ = min;

    // create manager thread
    manager_ = new std::thread(
        &ThreadPool::Manager,
        this
    );

    // create worker thread
    for(int i=0; i<current_thread_; ++i){
        std::thread t(&ThreadPool::Worker, this);  
        workers_.insert(std::make_pair(t.get_id(), move(t)));
    }
    
}

ThreadPool::~ThreadPool(){
    is_stop_ = true;
    condition_.notify_all();
    
    // thread object cannot be copyed
    for(auto& it:workers_){
        std::thread& t = it.second;
        if(t.joinable()){  
            t.join();
        }
    }

    // manager thread
    if(manager_->joinable()) 
        manager_->join();
    delete manager_;
}

void ThreadPool::Manager(){
    while(!is_stop_.load()){
        
        std::this_thread::
            sleep_for(std::chrono::seconds(2));
        int idle = idle_thread_.load();
        int current = current_thread_.load();

        if(idle > current / 2 && current > min_threads_number_){
            
            exit_thread_.store(2);
            condition_.notify_all();
            std::lock_guard<std::mutex> ids_locker(ids_mutex_);
            
            for(const auto& id:ids_){
                auto it = workers_.find(id);
                if(it != workers_.end()){
                    it->second.join();
                    workers_.erase(it);
                }
            }
            ids_.clear();

        }else if(0 == idle && current < max_threads_number_){

            std::thread t(&ThreadPool::Worker, this);
            workers_.insert(std::make_pair(
                t.get_id(), move(t)
            ));
    
            ++current_thread_;
            ++idle_thread_;
        }

    }
 
}

void ThreadPool::Worker(){
    while(!is_stop_.load()){
        std::function<void(void)> task = nullptr;
        
        {
            std::unique_lock<std::mutex> locker(queue_mutex_);
            
            while(tasks_.empty() && !is_stop_.load()){
                // block the thread and unlock queue_mutex_
                // because of avoiding deadlock
                condition_.wait(locker);

                if(exit_thread_ > 0){
                    --current_thread_;
                    --exit_thread_;
                    --idle_thread_;  
                    std::lock_guard<std::mutex> ids_locker(ids_mutex_);

                    ids_.emplace_back(
                        std::this_thread
                        ::get_id()
                    );
                    return;
                }
            }

            if(!tasks_.empty()){
                task = move(tasks_.front());
                tasks_.pop();
            }
        }

        if(task){
            --idle_thread_;
            task();
            ++idle_thread_;
        }
    }
}

void ThreadPool::AddTask(std::function<void(void)> task){
    {
        std::lock_guard<std::mutex> locker(queue_mutex_);
        tasks_.emplace(task);
    }
    condition_.notify_one();
}


}
