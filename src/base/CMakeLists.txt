set(base_SRCS
    AsyncLogging.cc
    Condition.cc
    CountDownLatch.cc
    CurrentThread.cc
    Date.cc
    Exception.cc
    FileUtil.cc
    LogFile.cc
    Logging.cc
    LogStream.cc
    ProcessInfo.cc
    TimeStamp.cc
    Thread.cc
    ThreadPool.cc
    TimeZone.cc
    Util.cc)

# add_library(mrpc_base ${base_SRCS})
add_library(mrpc_base SHARED ${base_SRCS})
target_link_libraries(mrpc_base pthread rt)

add_subdirectory(tests)