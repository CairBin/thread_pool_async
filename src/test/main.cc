#include "thread_pool_async/thread_pool.h"
#include <iostream>

using namespace std;
using namespace thread_pool_async;

int calc(int x, int y){
    int z = x+y; 
    this_thread::sleep_for(chrono::seconds(2));

    return z;
}

int main(){
    ThreadPool pool;
    vector<future<int>> result;
    for(int i=0; i<10; i++){
        result.emplace_back(
            pool.AddTask(calc, i, i*2)
        );
    }

    for(auto& item:result){
        cout << "Result: " << item.get() << endl;
    }
    
    return 0;
}
