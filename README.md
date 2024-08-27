# thread_pool_async

## 描述

基于 C++ 11 标准实现的异步线程池


## 编译

``` shell
mkdir build
cd build
cmake -DCMAKE_CXX_STANDARD=11 ..
make
```

## 用例

* 异步用法

```cpp
// calculate i+i*2
ThreadPool pool;
vector<future<int>> result;
for(int i=0; i<10; i++){
    result.emplace_back(
        pool.AddTask(CalcFunc, i, i*2)
    );
}
```

* 同步用法
```cpp
ThreadPool pool;
for(int i=0; i<10; i++){
    pool.AddTask(bind(CalcFunc, i, i*2));
}
```
