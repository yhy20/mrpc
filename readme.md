## 缘起

最初想根据教学视频实现一个 muduo + protobuf 的 RPC 调用服务，后来觉得只是使用而不知其所以然不太舒服，遂梳理、重写了 muduo 库，并修改了部分内容，目前 RPC 部分尚未完成，muduo 库框架部分已全部完成，所有的单元测试都已重写。


## base 模块

#### queue 目录的修改

(1)  ThreadSafeQueue 是参考《C++并发编程实践》实现的细粒度锁的线程安全队列

(2) ConcurrentQueue 是 github 上高性能的无锁线程安全队列实现，详见

(3) BlockingQueue 是 muduo 库原本的线程安全阻塞队列，接口按照 《C++并发编程实践》中的示例进行了修改

(4) BoundedBlockingQueue 是 muduo 库原本的固定容量线程安全阻塞队列，接口按照 《C++并发编程实践》中的示例进行了修改

#### Logging 中的修改

调整了不同日志级别的 C++ 宏实现以及日志的格式，编译时可以通过定义 `USE_FULL_FILENAME `宏来使用 `__FILE__` 的绝对路径名（默认使用 basename)，日志打印绝对路径名可以方便的使用 vscode 的快速跳转功能。同时提供了一组 C 风格日志宏，不过使用 C 风格日志宏导致无法使用高性能的日志流类，实测下来性能下降了 15% 左右，修改后的性能测试见 src/base/tests/Logging_test.cc

```C++
#define CLOG_TRACE(fmt, ...) if(mrpc::Logger::GetLogLevel() <= mrpc::Logger::TRACE) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::TRACE, __func__).format(fmt, ##__VA_ARGS__)
#define CLOG_DEBUG(fmt, ...) if(mrpc::Logger::GetLogLevel() <= mrpc::Logger::DEBUG) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::DEBUG, __func__).format(fmt, ##__VA_ARGS__)
#define CLOG_INFO(fmt, ...)  if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::INFO) \
    mrpc::Logger(__FILE__, __LINE__).format(fmt, ##__VA_ARGS__)
#define CLOG_WARN(fmt, ...) if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::WARN) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::WARN).format(fmt, ##__VA_ARGS__)
#define CLOG_ERROR(fmt, ...) if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::ERROR) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::ERROR).format(fmt, ##__VA_ARGS__)
#define CLOG_SYSERR(fmt, ...) if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::SYSER) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::SYSER).format(fmt, ##__VA_ARGS__)
#define CLOG_FATAL(fmt, ...) mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::FATAL).format(fmt, ##__VA_ARGS__)
#define CLOG_SYSFATAL(fmt, ...) mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::SYSFA).format(fmt, ##__VA_ARGS__)
```

#### Mutex 中的修改

将 muduo 原本的 Mutex 修改为 AssertMutex，添加了 Semaphore 类、Mutex 类和 SpinLock 类，并提供了配套的 RAII 模板类 LockGuard 和 UniqueLock，这些工具类和标准库提供工具类相互兼容，即 std::lock_guard、std::unique_lock、LockGuard、UniqueLock 和 std::mutex、Mutex、AssertMutex、SpinLock 之间可以混用，详细测试见 src/base/tests/Mutex_test.cc 和 src/base/tests/SemaphoreMute_test.cc

#### Condition 中的修改

考虑到目前可用的锁有四种，分别是 std::mutex、Mutex、AssertMutex 和 SpinLock，其中可以提供给 Condition 类使用的有 std::mutex、Mutex、AssertMutex 三种，故将 Condition 修改为模板类以兼容 3 种锁，关于 Condition 的测试采用了经典的 FooBar  程序，顺便也测试了 std::condition_variable 对 Mutex 和 AssertMutex 的兼容性，详细测试见 src/base/tests/Condition_test.cc

#### TimeStamp 中的修改

对 TimeStamp 类的改动比较小，主要是修改了运算符重载方式，在测试中添加的对所有重载方式正确性的测试，详解 src/base/tests/TimeStamp_test.cc

#### TimeZone 中的修改

时区类应该是 muduo 库 base 模块下最不容易理解的类，问题的本质是 Linux 系统下 localtime(2), localtime_r(2) 等调用都依赖于当前系统的时区设置，muduo 库希望提供给用于独立于系统时区的时区类，用户可以通过 /usr/share/zoneinfo/ 目录下的时区文件创建时区类以使用任何时区，而不受系统的限制。关于时区，一切的复杂性都源于夏令时，这也是 gettimeofday(2) 系统调用第二个时区参数被弃用的原因。对于不施行夏令时政策的国家或地区（例如中国）获取地区时间非常简单，只需要 UTC 时间 + 时差即可。对于实行夏令时政策的国家或地区，该政策会由当地的情况动态调整，可能在某个年份停止夏令时政策了，又可能几年后恢复了该政策。这意味着对于过去各地区的夏令时政策需要记录，如果我们使用过去的某个时刻，必须根据过去的夏令时政策进行 UTC 时间至地区时间的转化。而未来各个地区的夏令时政策（由人文，国家国情决定）也无法预测，必须由专门的机构来维护和更新。关于时区类，没有什么可以修改的部分，不过在时区类测试中可以详细的对时区进行理解，首通过修改系统时区随机生成下列时区文件 1970 至 2021 年约 50 组测试样例数据。

```c++
/usr/share/zoneinfo/Asia/Shanghai
/usr/share/zoneinfo/America/New_York
/usr/share/zoneinfo/Europe/London
/usr/share/zoneinfo/Asia/Hong_Kong
/usr/share/zoneinfo/Australia/Sydney
```

然后对比修改系统时区后生成的正确地区时间和手写时区类从 UTC 转化至时区文件对应地区的时间是否相同来验证程序的正确性，目前看来 1970 至 2021 年的 5 * 50 共 250 组数据全部通过了测试，详细测试见 src/base/tests/TimeZone_test.cc

#### ThreadPool 中的修改

线程池类并无可修改的地方，原理也容易理解，即简单的将任务压入线程任务队列，多个线程竞争锁来获取任务，并在各自线程执行，需要注意的是合理的选择线程数目和设置线程任务队列的容量（因为线程任务队列满后会阻塞），一般来说线程数目设置为硬件线程数较好，可用通过 std::thread::hardware_concurrency() 来获取，至于线程任务队列容量需要根据实际情况来决定，如果不限制容量，也可能造成任务积累，导致大量占用内存最后进程崩溃。测试代码中测试了多种情况下线程池的工作情况，部分测试代码如下：

```c++
void Test1()
{
    Util::PrintTitle("Test1");
    /// 任务队列容量无上限，同步执行
    BasicTest(-1, 0);
    /// 任务队列容量无上限，单线程执行
    BasicTest(-1, 1);
    /// 任务队列容量无上限，5个线程同时执行
    BasicTest(-1, 5);
    /// 任务队列容量无上限，10个线程同时执行
    BasicTest(-1, 10);
    /// 任务队列容量无上限，50个线程同时执行
    BasicTest(-1, 50);
}

void Test2()
{
    Util::PrintTitle("Test2");
    /// 任务队列容量为 1，同步执行
    BasicTest(1, 0);
    /// 任务队列容量为 1，单线程执行
    BasicTest(1, 1);
    /// 任务队列容量为 1，5个线程同时执行
    BasicTest(1, 5);
    /// 任务队列容量为 1，10个线程同时执行
    BasicTest(1, 10);
    /// 任务队列容量为 1，50个线程同时执行
    BasicTest(1, 50); 
}

void Test3()
{
    Util::PrintTitle("Test3");
    /// 任务队列容量无上限，5个线程同时执行
    BasicTest(0, 5);
    /// 任务队列容量为 1，5个线程同时执行
    BasicTest(1, 5);
    /// 任务队列容量为 5，5个线程同时执行
    BasicTest(5, 5);
    /// 任务队列容量为 10，5个线程同时执行
    BasicTest(10, 5);
    /// 任务队列容量为 50，5个线程同时执行
    BasicTest(50, 5);
}
```

#### Atomic 的修改
