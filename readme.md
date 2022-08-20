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

考虑到目前可用的锁有四种、std::mutex、Mutex、AssertMutex 和 SpinLock，其中可以提供给 Condition 类使用的有 std::mutex、Mutex、AssertMutex 三种，故将 Condition 修改为模板类以兼容 3 种锁，关于 Condition 的测试采用了经典的 FooBar  程序，顺便也测试了 std::condition_variable 对 Mutex 和 AssertMutex 的兼容性，详细测试见 src/base/tests/Condition_test.cc

#### TimeStamp 中的修改

对 TimeStamp 类的改动比较小，主要是修改了运算符重载方式，在测试中添加的对所有重载方式正确性的测试，详解 src/base/tests/TimeStamp_test.cc
