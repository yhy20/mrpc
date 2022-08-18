#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

#include "Logging.h"
#include "AsyncLogging.h"

using namespace mrpc;

const off_t g_rollSize = 500 * 1000 * 1000;

AsyncLogging* g_asyncLog = nullptr;

void AsyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

void Benchmark1(bool longLog)
{
    Logger::SetOutput(AsyncOutput);
    const int n = 1000 * 1000;

    std::string empty = " ";
    std::string longStr(3000, 'X');
    longStr += " ";

    TimeStamp start(TimeStamp::Now());
    for(int i = 0; i < n; ++i)
    {
        LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
                 << (longLog ? longStr : empty)
                 << i + 1;
    }
    TimeStamp stop(TimeStamp::Now());
    printf("%f\n", TimeDifference(stop, start));
}

void Benchmark(bool longLog)
{
    Logger::SetOutput(AsyncOutput);
    int cnt = 0;
    const int kBatch = 1000;
    std::string empty = " ";
    std::string longStr(3000, 'X');
    longStr += " ";

    for (int t = 0; t < 30; ++t)
    {
        TimeStamp start = TimeStamp::Now();
        for (int i = 0; i < kBatch; ++i)
        {
            LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
                    << (longLog ? longStr : empty)
                    << cnt + 1;
            ++cnt;
        }
        TimeStamp stop = TimeStamp::Now();
        printf("%f\n", TimeDifference(stop, start) /** 1000000 / kBatch*/);
        struct timespec ts = { 0, 500 * 1000 * 1000 };
        nanosleep(&ts, NULL);
    }
}

/// TODO: 完整测试
int main()
{
    {
        // set max virtual memory to 2GB.
        size_t kOneGB = 1000 * 1024 * 1024;
        rlimit rl = { 2 * kOneGB, 2 * kOneGB };
        setrlimit(RLIMIT_AS, &rl);
    }

    printf("pid = %d\n", getpid());
    AsyncLogging log("./async", g_rollSize);
    log.start();
    g_asyncLog = &log;
    // Benchmark(true);
    Benchmark1(false);
    return 0;
}



