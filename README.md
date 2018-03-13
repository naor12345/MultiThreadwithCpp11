# MultiThreadwithCpp11
A thread pool developed with cpp11

一个完备的多线程队列至少一把锁、两个信号量。一把锁锁住push和pop自身和彼此的竞争，两个信号量表示当队列空时和队列满时应该触发的动作。

本任务队列不限长度，故一把锁一个信号量；为了实现任务清空的功能，额外加入一把锁和一个信号量。


## 成员变量
- ```vector<thread> _pool```： 线程池
- ```queue<Task> _tasks```：任务队列
- ```mutex _lock```：互斥量，pop、push锁
- ```mutex _drain_lock;``：互斥量，用于任务清空
- ```condition_variable _task_cv```：信号量，队列空时触发
- ```condition_variable _drain_cv```：信号量，任务清空时触发
- ```atomic<bool> _run{ true }```：原子布尔，判定是否运行
- ```atomic<int>  _idlThrNum{ 0 }```：原子整型，保存当前空闲线程数
- ```atomic<int>  _totalThrNum{ 0 }```：原子整型，保存总线程数

## 成员函数
核心的函数有```void addThread()```和```auto commit() ```，用于构造时加入线程和向任务队列内提交任务。
### void addThread()
本函数关键在于加入线程池的线程的运行逻辑。

当线程池运行时，进入循环：从队列中取出任务，执行。取任务时用锁进行控制，当队列空时，挂起并等待对应信号量（信号量唤醒在```auto commit() ```里）

### auto commit()
本函数用于提交任务，注意一下提交形式：
```c++
commit(std::bind(&Dog::sayHello, &dog));
commit(std::mem_fn(&Dog::sayHello), this)
```

运行逻辑：
- 1. 获得函数返回类型：
```c++
using RetType = decltype(f(args...));
```
- 2. 把函数封装成任务：
```c++
auto task = make_shared<packaged_task<RetType()>>(
	bind(forward<F>(f), forward<Args>(args)...));
// bind：把函数名和参数绑定
// forward：完美转发
// packaged_task：封装函数
// make_shared：返回shared智能指针，可自动析构
```
- 3. 获取对应future，用于返回
```c++
future<RetType> future = task->get_future();
```
- 4. push入队列，用锁保护
```c++
lock_guard<mutex> lock{ _lock };
_tasks.emplace([task](){(*task)();});
```

- 5. 唤醒被队列空挂起的线程（如果有）
```c++
_task_cv.notify_one();
```
- 6. 返回对应future 