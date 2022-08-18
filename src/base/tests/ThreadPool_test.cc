#include <stdio.h>

#include <iostream>
#include <sstream>

#include "Util.h"
#include "Time.h"
#include "ThreadPool.h"
#include "CountDownLatch.h"
#include "CurrentThread.h"

using namespace mrpc;

void Print()
{
    printf("Tid = %d\n", CurrentThread::Tid());
}

void ShortTask(const std::string& str)
{
    // std::cout << str << std::endl;
    Util::SleepMsec(100);
}

/**
 * 
 * @param[in] maxSize 任务队列容量， maxSize <= 0 表示任务队列容量无上限
 * @param[in] threadNum 线程池中线程的数量
 */
void BasicTest(int maxSize, int threadNum)
{
    std::cout << "maxSize = " << maxSize << ", threadNum = " << threadNum << std::endl;
    ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(threadNum);

    // pool.run(Print);
    // pool.run(Print);

    std::cout << "Adding" << std::endl;
    WallTime timer;
    timer.start();
    for (int i = 0; i < 100; ++i)
    {
      char buf[32];
      snprintf(buf, sizeof(buf), "task %d", i);
      pool.run(std::bind(ShortTask, std::string(buf)));
    }

    /// 等待任务队列的全部任务运行结束
    CountDownLatch latch(1);
    pool.run(std::bind(&CountDownLatch::countDown, &latch));
    latch.wait();

    timer.stop();
    pool.stop(); 
    std::cout << "duration: " << timer.duration() << std::endl;
    std::cout << "Done" << std::endl;
    
}

void LongTask(int num)
{
    std::cout << "LongTask" << std::endl;
    Util::SleepSec(3);
}

void TestSpecialCase()
{
    Util::PrintTitle("Test ThreadPool by stoping early");
    ThreadPool pool("ThreadPool");
    pool.setMaxQueueSize(5);
    pool.start(3);

    Thread t([&pool]()
    {
        for (int i = 0; i < 20; ++i)
        {
            pool.run(std::bind(LongTask, i));
        }
    }, "thread1");

    t.start();

    Util::SleepSec(5);
    std::cout << "stop pool" << std::endl;
    /// early stop
    pool.stop();  
    t.join();
    /// run() after stop()
    pool.run(Print);
    std::cout << "Test2 Done!" << std::endl;
}

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

int main()
{ 
    // Test1();
    // Test2();
    // Test3();
    TestSpecialCase();

    return 0;
}

