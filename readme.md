## 缘起

最初想根据教学视频实现一个 muduo + protobuf 的 RPC 调用服务，后来觉得只是使用而不知其所以然不太舒服，遂梳理、重写了 muduo 库，并修改了部分内容，目前 RPC 部分尚未完成，muduo 库框架部分已全部完成，所有的测试单元都已重写。

## base 模块

#### queue 目录的修改

(1) `ThreadSafeQueue `是参考《C++并发编程实践》实现的细粒度锁的线程安全队列

(2) `ConcurrentQueue`是 github 上高性能的无锁线程安全队列实现，详见

(3) `BlockingQueue`是 muduo 库原本的线程安全阻塞队列，接口按照 《C++并发编程实践》中的示例进行了修改

(4) `BoundedBlockingQueue` 是 muduo 库原本的固定容量线程安全阻塞队列，接口按照 《C++并发编程实践》中的示例进行了修改

#### Logging 中的修改

调整了不同日志级别的 C++ 宏实现以及日志的格式，编译时可以通过定义 `USE_FULL_FILENAME `宏来使用 `__FILE__` 的绝对路径名（默认使用 basename)，日志打印绝对路径名可以方便的使用 vscode 的快速跳转功能。下列是开启 `USE_FULL_FILENAME `宏后便于统计和观察的日志格式，从左到右分别是日志级别、时间戳、线程 id、源文件和行号、日志信息（`LOG_ERROR、LOG_FATAL`、`LOG_SYSERR`、`LOG_SYSFATAL` 这四种日志还有额外的错误信息）

```c++
[INFO ][2022-08-21 01:11:01:171954Z][ 1895][/home/yhy/mrpc/example/net/asio/client.cc:54]127.0.0.1:41246 -> 127.0.0.1:5000 is UP
```

同时提供了一组 C 风格日志宏，可以像 `CLOG_INFO("num = %d\n", num)` 这样使用这些宏。

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

不过 C 风格日志宏的实现使用的是 `va_list` 和 `vsnprintf `函数，这导致无法使用 muduo 库实现的高性能日志流类，所以性能有所下降，下列是 `O2` 优化下的一组性能测试（每组测试中写 100 w 条日志），其中 nop 表示直接丢弃日志，fd 表示写到 `/dev/null` 对应的 fd 文件描述符， FILE 表示写到 `/dev/null` 对应的 FILE 流，两者的区别其实很简单，即 FILE 流多了一层应用层缓冲，从性能上也可以看出 FILE 的 462.55 MiB/s 明显比 fd 的 317.75 MiB/s 快的多。/tmp/log 表示使用 FILE 流写到 /tmp 目录下的 log 日志文件，这是真实情况下的写日志性能，test_log_st 表示使用 muduo 提供的同步日志后端类将日志信息非线程安全的写到当前目录下的 test_log_st 文件，而 test_log_mt 表示使用 muduo 提供的同步日志后端类将日志信息线程安全（即加锁）的写到当前目录下的 test_log_mt 文件。详细代码见 `src/base/tests/Logging_test.cc`

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

由上结果可以看出，

#### Mutex 中的修改

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

#### Condition 中的修改

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

#### TimeStamp 中的修改

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

#### TimeZone 中的修改

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

#### ThreadPool 中的修改

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

#### Atomic 中的修改

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

#### CAsyncLog 类

`CAsyncLog` 是一个基于 `ThreadSafeQueue` 实现轻量的备用 C 风格异步日志类，可以满足写测试程序，debug 文件等一些简单的需求，该类是由开源软件 flamingo 的异步日志修改而来。

## net 模块

## **learn 目录**

## todo
