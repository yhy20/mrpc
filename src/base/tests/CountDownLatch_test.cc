#include <vector>
#include <thread>
#include <iostream>

#include "Util.h"
#include "StringPiece.h"
#include "CountDownLatch.h"

using namespace mrpc;

int main()
{
    Util::PrintTitle("Test CountDownLatch");
    const int threadNum = 10;
    Mutex mtx;
    CountDownLatch latch(threadNum);
    auto threadTask = [&](int num)->void {
        Util::SleepSec(3);
        latch.countDown();

        {
            LockGuard<Mutex> lock(mtx);
            std::cout << "Thread" << num << " countDown !\n";
        }
    };
    
    std::vector<std::thread> threads;
    threads.reserve(threadNum);
    for(int i = 0; i < threadNum; ++i)
    {
        threads.push_back(std::thread(threadTask, i + 1));
    } 

    std::cout << "Wait for countDown!" << std::endl;
    latch.wait();

    {
        LockGuard<Mutex> lock(mtx);
        std::cout << "Main thread run!\n";
    }

    for(int i = 0; i < threadNum; ++i)
    {
        threads[i].join();
    }

    return 0;
}