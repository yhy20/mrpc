## 缘起

最初想根据教学视频实现一个 muduo + protobuf 的 RPC 调用服务，后来觉得只是使用而不知其所以然不太舒服，遂梳理、重写了 muduo 库，并修改了部分内容，目前 RPC 部分尚未完成，muduo 库框架部分已全部完成，大多数的测试单元都已重写。

## base 模块

### queue 目录

#### ThreadSafeQueue

`ThreadSafeQueue` 类是参考《C++并发编程实践》实现的细粒度锁版的线程安全队列，该队列用在了后续实现的 `CAsyncLog `类中，`ThreadSafeQueue` 类在入队方面提供了值拷贝的 `push` 入队函数和右值引用移动的 `movePush` 入队函数，在出队方面提供了阻塞出队和非阻塞出队两类共四组函数，并依据线程安全数据结构的惯例将 `top`与 `pop`这一对天然 race condition 的接口合并了，这些函数的原型分别是：

```c++
/// 使用拷贝构造函数 copy 传参进行 push
void push(T data);
/// 使用右值引用 move 传参进行 push  
void movePush(T&& data);
/// 阻塞的 pop 函数，使用引用返回
void waitAndPop(T& data);   
/// 阻塞的 pop 函数，使用 shared_ptr 返回
std::shared_ptr<T> waitAndPop();
/// 非阻塞的 pop 函数，使用引用返回
bool tryPop(T& data);   
/// 非阻塞的 pop 函数，使用 shared_ptr 返回
std::shared_ptr<T> tryPop(); 
```

在思考后，最终保留了 `empty`函数，这意味着经典问题 `empty `与 `pop` 的 race condition 没有解决，用户依然有使用错误的可能。一般情况下使用线程安全数据结构是为了不在外部使用其他的锁，所以下面是一种使用队列可能的情况。

```c++
void ThreadTask()
{
    if(!queue.empty())
    {
        DataType data;
        queue.waitAndPop(data);

        /// do some work on data.
    }
}
```

这段代码展示了 `empty` 与 `pop` 接口同时使用的一种经典错误，首先在单线程条件下运行肯定没有问题，但试想一种情况，队列中仅有一个 `data `资源，10 个线程同时运行该任务，最终结果是只有一个线程能够成功获取资源，执行任务并结束线程，其余九个线程（线程调用了 `join` 等待）都会被 `waitAndPop` 函数永远阻塞。不过多数情况下，服务器 `queue` 队列的数据会源源不断的到来，我们只需要使用 `waitAndPop` 一个函数来在业务线程中获取数据即可，如果服务器上需要处理的数据庞大，`queue `队列积累资源速度非常快，可以考虑使用 `busy-loop` + 非阻塞的 `tryPop` 来出队或直接使用无锁实现的 `queue `来分发资源。

#### ConcurrentQueue

`ConcurrentQueue`类是 github 上高性能的无锁线程安全队列实现，详见[https://github.com/cameron314/concurrentqueue]()

#### BlockingQueue

`BlockingQueue`是 muduo 库原本的线程安全阻塞队列，该类使用条件变量和粗粒度的锁实现，接口按照 《C++并发编程实践》中的示例进行了修改。

#### BoundedBlockingQueue

`BoundedBlockingQueue`类是 muduo 库原本的固定容量线程安全阻塞队列，该类在 `boost::circular_buffer` 的基础上使用条件变量和粗粒度的锁实现在，接口按照 《C++并发编程实践》中的示例进行了修改。

### Logging 中的修改

调整了不同日志级别的 C++ 宏实现以及日志的格式，编译时可以通过定义 `USE_FULL_FILENAME `宏来使用 `__FILE__` 的绝对路径名（默认使用 basename)，日志打印绝对路径名可以方便的使用 vscode 的快速跳转功能。下列是开启 `USE_FULL_FILENAME `宏后便于统计和观察的日志格式，从左到右分别是日志级别、时间戳、线程 id、源文件和行号、日志信息（`LOG_ERROR、LOG_FATAL`、`LOG_SYSERR`、`LOG_SYSFATAL` 这四种日志还有额外的错误信息）

```c++
[INFO ][2022-08-21 01:11:01:171954Z][ 1895][/home/yhy/mrpc/example/net/asio/client.cc:54]127.0.0.1:41246 -> 127.0.0.1:5000 is UP
```

同时提供了一组 C 风格日志宏，可以像 `CLOG_INFO("num = %d\n", num);` 这样使用这些宏。

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

不过 C 风格日志宏的实现使用的是 `va_list` 和 `vsnprintf` 函数，这导致无法使用 muduo 库实现的高性能日志流类，所以性能有所下降，下列是 `O2` 优化下的一组性能测试（每组测试中写 100 w 条日志），其中 nop 表示直接丢弃日志，fd 表示写到 `/dev/null `对应的 fd 文件描述符， FILE 表示写到 `/dev/null` 对应的 `FILE` 流，两者的区别其实很简单，即 `FILE` 流多了一层应用层缓冲，从性能上也可以看出 FILE 的 462.55 MiB/s 明显比 fd 的 317.75 MiB/s 快的多。/tmp/log 表示使用 FILE 流写到 /tmp 目录下的 log 日志文件，这是真实情况下的写日志性能，test_log_st 表示使用 muduo 提供的同步日志后端类 `LogFile` 将日志信息非线程安全的写到当前目录下的 `test_log_st` 文件，而 test_log_mt 表示使用 muduo 提供的同步日志后端类 `LogFile`  将日志信息线程安全（即加锁）的写到当前目录下的 `test_log_mt` 文件。详细代码见 `src/base/tests/Logging_test.cc`

```C++
/// C++ 风格日志宏的输出测试代码
/// LOG_INFO << "LOG Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz" << i + 1;
         nop: g_total = 144888896 bytes, fd_total = 0 bytes, fp_total = 0 bytes
       speed: duration = 0.269829 seconds, 3706050.87 msg/s, 512.09 MiB/s
          fd: g_total = 144888896 bytes, fd_total = 144888896 bytes, fp_total = 0 bytes
       speed: duration = 0.434862 seconds, 2299580.10 msg/s, 317.75 MiB/s
        FILE: g_total = 144888896 bytes, fd_total = 0 bytes, fp_total = 144888896 bytes
       speed: duration = 0.298731 seconds, 3347493.23 msg/s, 462.55 MiB/s
    /tmp/log: g_total = 144888896 bytes, fd_total = 0 bytes, fp_total = 144888896 bytes
       speed: duration = 0.414903 seconds, 2410201.90 msg/s, 333.03 MiB/s
 test_log_st: g_total = 144888896 bytes, fd_total = 0 bytes, fp_total = 0 bytes
       speed: duration = 0.410824 seconds, 2434132.38 msg/s, 336.34 MiB/s
 test_log_mt: g_total = 144888896 bytes, fd_total = 0 bytes, fp_total = 0 bytes
       speed: duration = 0.451881 seconds, 2212972.00 msg/s, 305.78 MiB/s
```

```c++
/// C 风格日志宏的输出测试代码
/// CLOG_INFO("CLOG Hello 0123456789 abcdefghijklmnopqrstuvwxyz %d", i + 1); 
         nop: g_total = 145888896 bytes, fd_total = 0 bytes, fp_total = 0 bytes
       speed: duration = 0.365384 seconds, 2736846.71 msg/s, 380.78 MiB/s
          fd: g_total = 145888896 bytes, fd_total = 145888896 bytes, fp_total = 0 bytes
       speed: duration = 0.526957 seconds, 1897688.05 msg/s, 264.03 MiB/s
   /dev/null: g_total = 145888896 bytes, fd_total = 0 bytes, fp_total = 145888896 bytes
       speed: duration = 0.399485 seconds, 2503222.90 msg/s, 348.27 MiB/s
    /tmp/log: g_total = 145888896 bytes, fd_total = 0 bytes, fp_total = 145888896 bytes
       speed: duration = 0.463908 seconds, 2155599.82 msg/s, 299.91 MiB/s
 test_log_st: g_total = 145888896 bytes, fd_total = 0 bytes, fp_total = 0 bytes
       speed: duration = 0.502663 seconds, 1989404.43 msg/s, 276.79 MiB/s
 test_log_mt: g_total = 145888896 bytes, fd_total = 0 bytes, fp_total = 0 bytes
       speed: duration = 0.651811 seconds, 1534187.06 msg/s, 213.45 MiB/s
```

以上结果可以看出，使用 `va_list` 和 `vsnprintf` 实现的 C 风格日志明显慢了许多，也变相的反映了 muduo 库的日志流类的优化，实际上日志流类的优化主要在下面这几个方面：

1. 日志流类定制的固定容量 FixedBuffer 类
2. Efficient Integer to String Conversions by Matthew Wilson.
3. 除了用户输入日志信息外的所有日志相关的字符串长度在编译期间计算

### Mutex 中的修改

将 muduo 原本的 `Mutex` 修改为 `AssertMutex`，添加了 `Semaphore` 类、`Mutex` 类和 `SpinLock `类，并提供了配套的 RAII 模板类 `LockGuard `和 `UniqueLock`，这些工具类和标准库提供工具类相互兼容，即 `std::lock_guard`、`std::unique_lock`、`LockGuard`、`UniqueLock` 和 `std::mutex`、`Mutex`、`AssertMutex`、`SpinLock` 之间可以混用，例如下列 `FooBar` 测试程序代码片段中直接向 `LockGuard` 和 `UniqueLock` 类模板传递四种类型的锁。详细代码见  `src/base/tests/Mutex_test.cc `和 `src/base/tests/Semaphore_test.cc`

```c++
void TestLockGuard()
{
    BasicTest<Mutex>("Test Mutex");
    BasicTest<std::mutex>("Test std::mutex");
    BasicTest<AssertMutex>("Test AssertMutex");
    BasicTest<SpinLock>("Test SpinLock");
}

void TestUniqueLock()
{
    BasicTest<Mutex, UniqueLock<Mutex>>("Test Mutex");
    BasicTest<std::mutex, UniqueLock<std::mutex>>("Test std::mutex");
    BasicTest<AssertMutex, UniqueLock<AssertMutex>>("Test AssertMutex");
    BasicTest<SpinLock, UniqueLock<SpinLock>>("Test SpinLock");
}
```

### Condition 中的修改

考虑到目前可用的锁有四种，分别是 `std::mutex`、`Mutex`、`AssertMutex` 和 `SpinLock`，其中可以提供给 `Condition` 类使用的有 `std::mutex`、`Mutex`、`AssertMutex` 三种，故将 `Condition` 修改为模板类以兼容 3 种锁，关于 `Condition` 的测试采用了经典的 `FooBar` 程序，顺便也测试了 `std::condition_variable `对 `Mutex` 和 `AssertMutex` 的兼容性，可以像下列代码片段一样使用 `Condition` 类，详细代码见 `src/base/tests/Condition_test.cc`

```c++
template <typename _Mutex, typename _Condition = Condition<_Mutex>>
void FooBarPrint(int num, StringArg title, bool conditionWait = false)
{   
    Util::PrintTitle(title);
    std::function<void(int)> printFoo = [](int i) -> void { printf("num%d: Foo\n", i); };
    std::function<void(int)> printBar = [](int i) -> void { printf("num%d: Bar\n", i); };
    FooBar<_Mutex, _Condition> obj(num, conditionWait);
    std::thread t1(&FooBar<_Mutex, _Condition>::foo, &obj, ref(printFoo));
    std::thread t2(&FooBar<_Mutex, _Condition>::bar, &obj, ref(printBar));
    t1.join();
    t2.join();
}

void TestFooBarPrint()
{
    FooBarPrint<Mutex>(10, "Test mrpc::Condition<mrpc::Mutex>");
    FooBarPrint<std::mutex>(10, "Test mrpc::Condition<std::mutex>");
    FooBarPrint<AssertMutex>(10, "Test mrpc::Condition<mrpc::AssertMutex>");
    FooBarPrint<std::mutex, std::condition_variable>(10, "Test std::condition_variable with wait");
    FooBarPrint<std::mutex, std::condition_variable>(10, "Test std::condition_variable with conditionWait", true);
}  
```

### TimeStamp 中的修改

对 `TimeStamp` 类的改动比较小，主要是修改了运算符重载方式并在测试中添加了对运算符重载函数的测试，不过 muduo 库中对于 `TimeStamp` 类的测试给了我一些启发，做法是一次性生成 1000w 组时间戳数据，然后计算相邻两个时间戳的差值并记录到对应下标的数组（数组大小为 100），若差值小于零则发生错误，差值大于等于 100 微妙则是 big gap，以此来测试 `TimeStamp` 类的性能和稳定性，一组测试的结果见下，详细代码见 `src/base/tests/TimeStamp_test.cc`

```c++

<------------Benchmark------------>
start time = 2022-08-21 10:00:59:240102
stop  time = 2022-08-21 10:00:59:431811
diff = 0.191709
 0: 9811278
 1: 187381
 2: 806
 3: 218
 4: 77
 5: 85
 6: 58
 7: 15
 8: 15
 9: 20
10: 21
11: 11
12: 5
13: 3
14: 1
15: 1
16: 0
...(后续都是 0)
```

从结果来看，本次测试的 1000w 组数据中约 98% 的相邻时间差小于 1 us，1.8% 的相邻时间差小于 2 us，其他则分别散落在 3 us ~ 16 us 之间，大于 16 us 几乎没有，错误和 big gap 并未发生，这在一定程度上证明了 `TimeStamp` 类的性能。

### TimeZone 中的修改

时区类应该是 muduo 库 base 模块下最不容易理解的类，问题的本质是 Linux 系统下 `localtime(2)`, `localtime_r(2)` 等调用都依赖于当前系统的时区设置，muduo 库希望提供给用户独立于系统时区的时区类，用户可以通过 `/usr/share/zoneinfo/`目录下的时区文件创建时区类以使用任何时区，而不受系统的限制。关于时区，一切的复杂性都源于夏令时，这也是 `gettimeofday(2)` 系统调用第二个时区参数被弃用的原因。对于不施行夏令时政策的国家或地区（例如中国）获取地区时间非常简单，只需要 UTC 时间 + 时差即可。对于实行夏令时政策的国家或地区，该政策会由当地的情况动态调整，可能在某个年份停止了夏令时政策，又可能几年后恢复了该政策。这意味着对于过去各地区的夏令时政策需要记录，如果我们使用过去的某个时刻，必须根据过去的夏令时政策进行 UTC 时间至地区时间的转化。而未来各个地区的夏令时政策（由人文，国家国情决定）也无法预测，必须由专门的机构来维护和更新。由上所述，想要实现独立于系统时区的时区类，必须要解析 tzfile 时区文件，不过时区文件并非人类可读的形式，通过 `zdump -v /etc/localtime` 可以查看可读的系统时区的历史信息，形式如下，可以看到时区文件中信息按照年份由低到高排序，且同一年不同季度的夏令时政策不同。

```c++
/etc/localtime  Mon Dec 31 15:54:16 1900 UTC = Mon Dec 31 23:59:59 1900 LMT isdst=0 gmtoff=29143
/etc/localtime  Mon Dec 31 15:54:17 1900 UTC = Mon Dec 31 23:54:17 1900 CST isdst=0 gmtoff=28800
/etc/localtime  Sat Apr 12 15:59:59 1919 UTC = Sat Apr 12 23:59:59 1919 CST isdst=0 gmtoff=28800
/etc/localtime  Sat Apr 12 16:00:00 1919 UTC = Sun Apr 13 01:00:00 1919 CDT isdst=1 gmtoff=32400
/etc/localtime  Tue Sep 30 14:59:59 1919 UTC = Tue Sep 30 23:59:59 1919 CDT isdst=1 gmtoff=32400
/etc/localtime  Tue Sep 30 15:00:00 1919 UTC = Tue Sep 30 23:00:00 1919 CST isdst=0 gmtoff=28800
/etc/localtime  Fri May 31 15:59:59 1940 UTC = Fri May 31 23:59:59 1940 CST isdst=0 gmtoff=28800
/etc/localtime  Fri May 31 16:00:00 1940 UTC = Sat Jun  1 01:00:00 1940 CDT isdst=1 gmtoff=32400
/etc/localtime  Sat Oct 12 14:59:59 1940 UTC = Sat Oct 12 23:59:59 1940 CDT isdst=1 gmtoff=32400
/etc/localtime  Sat Oct 12 15:00:00 1940 UTC = Sat Oct 12 23:00:00 1940 CST isdst=0 gmtoff=28800
/etc/localtime  Fri Mar 14 15:59:59 1941 UTC = Fri Mar 14 23:59:59 1941 CST isdst=0 gmtoff=28800
```

通过 `man 5 tzfile `或 `info tzfile `可以看到  man  手册对 tzfile 文件的描述，按照描述的格式可以取出要使用地区的历史时区信息，如果要使用某个地区过去的某个时刻，只需要在历史时区信息上作二分查找，找到该地区过去这一时刻对应的时区政策，再将 UTC 纪元时转化到地区时间即可。对于时区类本身，没有什么可以修改的地方，不过在时区类测试中可以详细的对时区进行理解，首先通过修改系统时区随机生成下列时区文件 1970 至 2021 年约 50 组测试样例数据。

```c++
/usr/share/zoneinfo/Asia/Shanghai
/usr/share/zoneinfo/America/New_York
/usr/share/zoneinfo/Europe/London
/usr/share/zoneinfo/Asia/Hong_Kong
/usr/share/zoneinfo/Australia/Sydney
```

下列是 `/usr/share/zoneinfo/America/New_York`时区文件的部分测试数据，从左到右分别是随机生成的 UTC 时间，修改系统时区后通过函数 `localtime_r(3)` 和 `strftime(3) `生成的正确的地区时间，以及当时是否实施夏令时政策。

```c++
{ "1970-01-12 13:46:40", "1970-01-12 08:46:40-0500(EST)", false },
{ "1971-01-29 06:53:20", "1971-01-29 01:53:20-0500(EST)", false },
{ "1972-02-15 00:00:00", "1972-02-14 19:00:00-0500(EST)", false },
{ "1973-03-02 17:06:40", "1973-03-02 12:06:40-0500(EST)", false },
{ "1974-03-19 10:13:20", "1974-03-19 06:13:20-0400(EDT)", true  },
{ "1975-04-05 03:20:00", "1975-04-04 23:20:00-0400(EDT)", true  },
{ "1976-04-20 20:26:40", "1976-04-20 15:26:40-0500(EST)", false },
{ "1977-05-07 13:33:20", "1977-05-07 09:33:20-0400(EDT)", true  },
{ "1978-05-24 06:40:00", "1978-05-24 02:40:00-0400(EDT)", true  },
```

先通过时区文件创建对应地区的时区类，然后将测试数据的 UTC 时间传递给时区类并直接转化得到地区时间和夏令时信息，然后将得到的结果与正确答案进行对拍即可。目前看来 1970 至 2021 年的 5 * 50 共 250 组数据全部通过了测试，详细测试见 `src/base/tests/TimeZone_test.cc`

### ThreadPool 中的修改

线程池类并无可修改的地方，原理也容易理解，即简单的将任务压入线程任务队列，多个线程竞争锁来获取任务，并在各自线程执行，需要注意的是合理的选择线程数目和设置线程任务队列的容量（因为线程任务队列满后会阻塞），一般来说线程数目设置为硬件线程数较好，可用通过 `std::thread::hardware_concurrency()` 来获取，至于线程任务队列容量需要根据实际情况来决定，如果不限制容量，也可能造成任务积累，导致大量占用内存最后进程崩溃。测试代码中测试了多种情况下线程池的工作情况，部分测试代码如下：

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

### Atomic 中的修改

`Atomic` 类是整数类型的原子变量类，目前将其接口修改的贴近 `std::atomic`，要简单的验证其是否正确，可以在 10 个线程中无锁的跑下列任务，然后对比其与 `std::atomic` 变量的结果即可。

```c++
 void threadTask()
    {
        for(int i = 0; i < 1000; ++i)
        {
            /// Atomic
            count.getAndAdd(2);
            count.addAndGet(2);
            count.incrementAndGet();
            count.decrementAndGet();
            count.increment();
            count.decrement();
            count.add(6);
	    /// std::atomic
            acount.fetch_add(10, std::memory_order_relaxed);
        }  
    }
```

### CAsyncLog 类

`CAsyncLog` 是一个基于 `ThreadSafeQueue` 实现轻量的备用 C 风格异步日志类，可以满足写测试程序，debug 文件等一些简单的需求，该类是由开源软件 flamingo 的异步日志修改而来，也是我实现的第一个异步日志类，关于 flamingo，见 [https://github.com/balloonwj/flamingo]()

#### AsyncLogging

## net 模块

### Channel 类中的问题讨论

#### (1) 多线程下 C++ 对象生命周期管理问题

C++ 中一个无法回避的问题是对象生命周期管理的问题，这个问题变相的也是内存管理问题，一般情况下通过 RAII 编程技法和智能指针能够解决大部分问题，不过在多线程情况下对象的析构更为复杂了，一方面如果一个线程可以被多个线程使用，则在一个线程调用过程中对象被另一个线程析构了将很可能导致程序崩溃，另一方面有时候需要延长对象的生命周期。muduo 库提供了一种解决思路，即 tie 函数，tie 实际上是一个冗余的 shared_ptr 指针，用于保护或延长特定对象的生命周期。在 muduo 中，当对端 TCP 连接断开时会触发 `Channel::handleEvent()` 调用，而 `handleEvent` 中会调用用户提供的 `CloseCallback`，而用户的代码在 `CloseCallback` 中可能会析构 `TcpConnection` 对象并导致 Channel 对象被析构，此时会造成 `Channel::handleEvent()` 执行到一半的时候，其所属的 Channel 对象本身被销毁了，这时程序会 core dump，而 tie 就可以起到保护作用。

#### (2) Epoll 全事件触发情况分析和测试

下列事件是既可用于 `epoll_ctl(2)` 注册也会被 `epoll_wait(2)` 作为 revents 返回的事件

1. `EPOLLIN`: The associated file is available for read(2) operations.
   一般作为读事件处理，通常意味着 fd 的内核读缓冲区中有数据，此时调用 `read(2)` 不会陷入阻塞状态
2. `EPOLLPRI`: There is urgent data available for read(2) operations.
   一般作为读事件处理，表示紧急数据到达，例如 tcp socket 的带外数据
3. `EPOLLOUT`: The associated file is available for write(2) operations.
   一般作为写事件处理，通常意味着 fd 的内核写缓冲区未满，此时调用 `write(2)` 不会陷入阻塞状态
4. `EPOLLRDHUP`: (since Linux 2.6.17) Stream socket peer closed connection, shut down writing half of connection or stream socket local shut down reading half of connectoin. This flag is especially useful for writing simple code to detect peer shutdownwhen using Edge Triggered monitoring.
   该事件只有在注册后才会触发，一般在水平触发模式下要监听对方是否关闭套接字，只需要监听 EPOLLIN 事件并调用 read 返回 0 即可。但在边缘触发模式下，如果不及时处理对方关闭套接字事件可能会导致永远丢失处理该事件。而在边缘触发模式下使用 `EPOLLRDHUP` 可以保证 `epoll_wait(2)` 会向水平触发一样，总是返回 `EPOLLRDHUP` 事件就绪，可以方便的进行延迟处理，而不会导致事件处理的丢失。
5. `EPOLLONESHOT`: (since Linux 2.6.2) sets the one-shot behavior for the associated file descriptor. This means that after an event is pulled out with epoll_wait(2) the associated file descriptor is internally disabled and no other events will be reported by the epoll interface.
   `EPOLLONESHOT` 表示一次性事件，在触发一次后就会失效，需要修改重新注册才能再次生效，通常用于和 `EPOLLOUT` 事件一同使用。在水平触发模式下，fd 内核缓冲区不满，`epoll_wait(2)` 会始终返回 `EPOLLOUT` 事件就绪，大量的积累会导致 busy-loop，白白浪费 CPU 时间片资源
6. EPOLLET: Sets the Edge Triggered behavior for the associated file descriptor.The default behavior for epoll is Level Triggered. See epoll(7) for more detailed information about Edge and Level Triggered event distribution architectures.
   将文件描述符设置为边缘触发，epoll 对 fd 的默认行为是条件触发。

下列事件是仅会被 `epoll_wait(2)` 作为 revents 返回的事件，它们会始终被 epoll 监听，不需要用户手动注册

1. `EPOLLERR`: Error condition happened on the associated file descriptor. epoll_wait(2) will always wait for this event; it is not necessary to set it in events.
   仅用于内核设置返回事件 revents，表示发生错误。只有采取动作时，才能知道是否对方异常。即客户端突然断掉是不会也不可能会主动触发 `EPOLLERR` 事件的。只有服务器采取动作（当然服务器此刻也不知道发生异常）read or write 发生错误时，会触发 `EPOLLERR` 事件，说明对方已经异常断开。触发 `EPOLLERR` 事件，一般的处理方法是将 socketfd 从 epollfd 中 DEL 再 `close` 即可。
   PS: 当客户端的机器异常崩溃了或者网络断掉了，则服务器一端是无从知晓的。发生这种情况与是否使用 epoll 无关，必须要服务器主动的去检查才能发现并解决问题。而服务器显然不可能经常的向客户写数据，通过有没有发生 `EPOLLERR` 事件来确认客户端是否有问题，因此服务器端的超时检查很重要。即使没有上述情况发生也应当做超时检查并主动断开连接，比如利用客户端恶意连接（只连接不发送数据）来攻击服务器，恶意占用服务器资源致使服务器崩溃。
2. EPOLLHUP: Hang up happened on the associated file descriptor. epoll_wait(2) will always wait for this event; it is not necessary to set it in events.
   man 手册对于 `EPOLLHUP` 事件的描述非常少，许多人误以为对端关闭 TCP 连接就叫 hang hup. 一部分是原因 pipe 的误导，如果将 pipe 的一端注册到 epollfd，关闭另一端会触发 `EPOLLHUP` 事件，不过对于 socketfd 则不一样，只有 `SHUT_WR` 和 `SHUT_RD` 同时关闭才会触发 `EPOLLHUP`。举个常见的例子，客户端调用 `shutdown` 或 `close` 关闭 TCP 的写连接，此时内核会发送 FIN 包至服务器，服务器的内核会回复一个 ACK 包，而服务器收到 FIN 报文也就意味着被动关闭了读连接（SHUT_RD)，此时服务器处于 CLOSE_WAIT 状态。当服务器需要发送的数据全部写到 socketfd 上后会主动调用 `shutdown` 或 `close` 关闭写连接（SHUT_WR)，此时满足了 hang up 的条件（需要注意此时数据在内核缓冲区上由内核代为发送，发送完毕后会发送 FIN 报文完成挥手），不过满足 hang up 条件并不表示会触发 `EPOLLHUP `事件，因为 close 引用计数为 1 的 socketfd 会直接将该 fd 从 epollfd 上清除，也就自然不会触发 `EPOLLHUP` 事件了，所以要触发 `EPOLLHUP`，服务器必须使用 `shutdown` 调用关闭连接。

Epoll 所有事件触发情况分析:

1. `EPOLLIN`: 在 epollfd 上注册该事件，对端数据正常到达或对端正常关闭
   `EPOLLIN` 事件触发的详细测试代码见 `src/learn/iomultiplexing/epoll_thread` 目录下的 EPOLLIN test1 和 test2
2. `EPOLLOUT`: 在 epollfd 上注册该事件，内核写缓冲区（缓冲区大小取决于 TCP 窗口和拥塞控制）空闲或 fd 非阻塞
   `EPOLLOUT` 事件触发的详细测试代码见 ` src/learn/iomultiplexing/epoll_thread` 目录下的 EPOLLOUT_test1.cc
3. `EPOLLRDHUP`: 在 epollfd 上注册该事件，对端正常关闭，本端调用 `shutdown(fd, SHUT_RD);`
   `EPOLLRDHUP` 事件触发的详细测试代码见 `src/learn/iomultiplexing/epoll_thread` 目录下 EPOLLRDHUP test1、test2 和 test3
4. `EPOLLHUP`: 监听的 socketfd 尚未建立连接、对端调用 `shutdown(fd, SHUT_WR)` 或 `close(fd)` 且本端调用 `shutdown(fd, SHUT_RD)`，本端调用 `shutdown(fd, SHUT_RDWR)` 或 `shutdown(fd, SHUT_RD)` + `shutdown(fd, SHUT_WR)`
   EPOLLHUP 事件触发的详细测试代码见 `src/learn/iomultiplexing/epoll_thread` 目录下 EPOLLHUP test1、test2、test3 和 test4

### Acceptor 中关于 fd 耗尽的问题

Read the section named "The special problem of accept()ing when you can't" in libev's doc by Marc Lehmann, author of libev. 原文的意思大致如下:

在大型服务器中，经过会出现描述符用完（通常是 linux 的资源限制导致的）的情况，这会导致 accept() 失败，并返还一个 ENFILE 错误，但是内核并没有拒绝这个连接，连接仍然在连接队列中，这导致在下一次迭代的时候，仍然会触发监听描述符的可读事件，最终造成程序 busy loop。一种简单的处理方式就是当程序遇到这种问题就直接忽略掉，直到这种情况消失，显然这种方法将会导致 `busy waiting`，一种比较好的处理方式就是记录除了 EAGAIN 或 EWOULDBLOCK 其他任何错误，告诉用户出现了某种错误，并停止监听描述符的可读事件，减少 CPU 的使用。如果程序是单线程的，我们可以先 open /dev/null，保留一个描述符，当 accept() 出现 ENFILE 或 EMFILE 错误的时候，close 掉 /dev/null 这个 fd，然后 accept，再 close 掉 accept 产生的 fd，然后再次 open /dev/null，这是一种比较优雅的方式来拒绝掉客户端的连接。最后一种方式则是遇到 accept() 的这种错误，直接拒绝并退出，但是显然这种方式很容易受到 Dos 攻击。而 Acceptor 对象仅在 Acceptor 线程中受理客户端连接，符合上述单线程优雅处理方式。

### EventLoop 中关于 SIGPIPE 信号的处理

一、SIGPIPE 信号产生原因以及为什么需要处理该信号。

1. 产生原因：当程序向一个接收了 RST 的套接字执行写操作时，会触发 SIGPIPE 信号。
2. 处理原因：SIGPIPE 信号的默认行为是终止进程，会导致整个服务器意外退出。

二、导致 TCP 发送 RST 报文的原因。

1. 发送 SYN 报文时指定的目的端口没有接收进程监听。
   这种情况下常见的例子是客户端访问服务器未监听的端口，服务器回复 RST 报文。比如，访问 Web 服务器的 21 端口（FTP），如果该端口服务器未开放或者阻断了到该端口的请求报文，则服务器很可能会给客户端 SYN 报文回应一个 RST 报文。因此，服务器对终端的 SYN 报文响应 RST 报文在很多时候可以作为判断目标端口是否开放的一个可靠依据。当然，在大多数场景下，服务器对到达自身未监听端口的报文进行丢弃而不响应是一种更为安全的实现。
2. 客户端尝试连接服务端的一个端口，其处于 TIME_WAIT 状态时，服务端会向客户端发 RST。
3. TCP 的一端主动发送 RST, 丢弃发送缓冲区数据，异常终止连接。
   正常情况下结束一个已有 TCP 连接的方式是发送 FIN，FIN 报文会在所有排队数据都发出后才会发送，正常情况下不会有数据丢失，因此这也被称为是有序释放。另外一种拆除已有 TCP 连接的方式就是发送 RST，这种方式的优点在于无需等待数据传输完毕，可以立即终结连接，这种通过 RST 结束连接的方式被称为异常释放。
4. 向特殊的半连接状态套接字发送数据会收到回复的 RST 报文（此处指的不是 shutdown 调用的半关闭状态）
   正常情况下 TCP 通过四元组标识一个已经创建的连接，当服务器或客户端收到一个新四元组（服务器或客户端本地没有这个连接）的非 SYN 首包就会丢弃该报文并回复一个 RST 报文。举个例子：位于不同机器上的用户端和服务器在正常连接的情况下，突然拔掉网线，再重启服务端。在这个过程中客户端感受不到服务端的异常，还保持着连接（此时就是半连接状态）客户端向该连接写数据，会收到服务端回复的 RST 报文，如果客户端再收到 RST 报文后继续向该连接写数据，会触发 SIGPIPE 信号，默认情况下会导致客户端异常退出。

三、在网络库中可能触发 SIGPIPE 信号的情况。
    假设服务器繁忙，没有及时处理对方断开连接的事件，就有可能出现在连接断开后继续发送数据的情况，下面的例子模拟了这种情况。

```c++
void onConnection(const TcpConnectionPtr& conn)
{
     if(conn->connected())
     {
         ::sleep(5);
         conn->send(message1); // 会收到 RST 报文
         conn->send(message2); // 触发 SIGPIPE 信号
     }  
}
```

**四、SIGPIPE 信号触发测试代码** 

详细的测试代码见 `src/learn/iomultiplexing/epoll_thread` 目录下的 SIGPIPE test1, test2，在测试代码中还讨论了 `shutdown(sock, SHUT_RD);` 的行为，及其平台相关性。 `shutdown(sock, SHUT_RD);` 的平台相关性会影响到 RST 报文的发送。


## **learn 目录**

## todo
