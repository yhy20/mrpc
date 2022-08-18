/// 所有代码只在 linux 下测试运行，不涉及跨平台
/// 所有后续的测试代码都只会在一个源文件中引用 header.h 
/// 故在头文件中定义全局变量和函数不会导致链接时的二义性

#ifndef __HEADER_H__
#define __HEADER_H__

#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <wchar.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <signal.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <string>
#include <thread>
#include <iostream>

#include "ename.c.inc"

#define GN_NONNEG       01      /* Value must be >= 0 */
#define GN_GT_0         02      /* Value must be > 0 */

                                /* By default, integers are decimal */
#define GN_ANY_BASE   0100      /* Can use any base - like strtol(3) */
#define GN_BASE_8     0200      /* Value is expressed in octal */
#define GN_BASE_16    0400      /* Value is expressed in hexadecimal */

namespace details
{

/**
 * Print a diagnostic message that contains a function name ('fname'),
 * the value of a command-line argument ('arg'), the name of that
 * command-line argument ('name'), and a diagnostic error message ('msg').
 */
void GnFail(const char *fname, const char *msg, const char *arg, const char *name)
{
    fprintf(stderr, "%s error", fname);
    if (name != NULL)
        fprintf(stderr, " (in %s)", name);
    fprintf(stderr, ": %s\n", msg);
    if (arg != NULL && *arg != '\0')
        fprintf(stderr, "        offending text: %s\n", arg);

    exit(EXIT_FAILURE);
}

/** 
 * Convert a numeric command-line argument ('arg') into a long integer,
 * returned as the function result. 'flags' is a bit mask of flags controlling
 * how the conversion is done and what diagnostic checks are performed on the
 * numeric result; see get_num.h for details.
 *
 * 'fname' is the name of our caller, and 'name' is the name associated with
 * the command-line argument 'arg'. 'fname' and 'name' are used to print a
 * diagnostic message in case an error is detected when processing 'arg'. 
 */
long GetNum(const char *fname, const char *arg, int flags, const char *name)
{
    long res;
    char *endptr;
    int base;

    if (arg == NULL || *arg == '\0')
        GnFail(fname, "null or empty string", arg, name);

    base = (flags & GN_ANY_BASE) ? 0 : (flags & GN_BASE_8) ? 8 :
                        (flags & GN_BASE_16) ? 16 : 10;

    errno = 0;
    res = strtol(arg, &endptr, base);
    if (errno != 0)
        GnFail(fname, "strtol() failed", arg, name);

    if (*endptr != '\0')
        GnFail(fname, "nonnumeric characters", arg, name);

    if ((flags & GN_NONNEG) && res < 0)
        GnFail(fname, "negative value not allowed", arg, name);

    if ((flags & GN_GT_0) && res <= 0)
        GnFail(fname, "value must be > 0", arg, name);

    return res;
}

}  // namespace details

/* Convert a numeric command-line argument string to a long integer. See the
 * comments for getNum() for a description of the arguments to this function.
 */
long GetLong(const char *arg, int flags, const char *name)
{
    return details::GetNum("getLong", arg, flags, name);
}

/* Convert a numeric command-line argument string to an integer. See the
 * comments for getNum() for a description of the arguments to this function.
 */
int GetInt(const char *arg, int flags, const char *name)
{
    long res = details::GetNum("getInt", arg, flags, name);

    if (res > INT_MAX || res < INT_MIN)
        details::GnFail("getInt", "integer out of range", arg, name);

    return res;
}

enum LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

namespace details
{

const char* kLogLevelName[int(LogLevel::FATAL + 1)] =
{
    "TRACE",
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
    "FATAL"
};

void Terminate(bool useExit3)
{
    char *s;
    /* dump core if EF_DUMPCORE environment variable is defined and
     * is a nonempty string; otherwise call exit(3) or _exit(2),
     * depending on the value of 'useExit3'. 
     */
    s = getenv("EF_DUMPCORE");
    
    if (s != NULL && *s != '\0')
        abort();
    else if (useExit3)
        exit(EXIT_FAILURE);
    else
        _exit(EXIT_FAILURE);
}

LogLevel g_logLevel = LogLevel::INFO;

}  // namespace details

void SetLogLevel(LogLevel level)
{
    details::g_logLevel = level;
}

LogLevel GetLogLevel()
{
    return details::g_logLevel;
}

#define LOG_TRACE(...) if(LogLevel::TRACE >= GetLogLevel()) \
    Output(LogLevel::TRACE, __FILE__, __LINE__, errno, true, true, ##__VA_ARGS__)
#define LOG_DEBUG(...) if(LogLevel::DEBUG >= GetLogLevel()) \
    Output(LogLevel::DEBUG, __FILE__, __LINE__, errno, true, true, ##__VA_ARGS__)
#define LOG_INFO(...) if(LogLevel::INFO >= GetLogLevel()) \
    Output(LogLevel::INFO, __FILE__, __LINE__, errno, true, true, ##__VA_ARGS__)
#define LOG_WARN(...) if(LogLevel::WARN >= GetLogLevel()) \
    Output(LogLevel::WARN, __FILE__, __LINE__, errno, true, true, ##__VA_ARGS__)
#define LOG_ERROR(...) if(LogLevel::ERROR >= GetLogLevel()) \
    Output(LogLevel::ERROR, __FILE__, __LINE__, errno, true, true, ##__VA_ARGS__)
/// 打印保存的 savedErrno 的相关错误信息
#define LOG_ERROR_SE(savedErrno, ...) if(LogLevel::ERROR >= GetLogLevel()) \
    Output(LogLevel::ERROR, __FILE__, __LINE__, savedErrno, true, true, ##__VA_ARGS__)
#define LOG_FATAL(...) if(LogLevel::FATAL >= GetLogLevel()) \
    Output(LogLevel::FATAL, __FILE__, __LINE__, errno, true, true, ##__VA_ARGS__)
/**
 * LOG_FATAL_EX 基于下述原因实现（摘自 tlpi_hdr.h -> error_funcitons.c -> err_exit()）
 * err_exit() especially useful in a library
 * function that creates a child process that must then terminate
 * because of an error: the child must terminate without flushing
 * stdio buffers that were partially filled by the caller and without
 * invoking exit handlers that were established by the caller.
 * 
 * 不刷新 stdout 并且使用 _exit(2) 代替 exit(3) 退出
 * 对于子父进程共享的 fd，可以防止子进程结束时，子进程刷新了父进程正在写的缓冲区
 * 测试中无法在子进程 fflush 子父进程共享的 fd 的缓冲区，存疑？？？ 可能与操作系统版本有关
 * 测试程序见 header_test/log_fatal_ex_test.cc
 * TODO: 探究测试中发现问题的原因（apue, tlpi)
 */
#define LOG_FATAL_EX(...) if(LogLevel::FATAL >= GetLogLevel()) \
    Output(LogLevel::FATAL, __FILE__, __LINE__, errno, false, true, ##__VA_ARGS__)

void Output(LogLevel level,
            const char* fileName,
            int lineNumber,
            int savedErrno,
            bool flushStdout,
            bool useExit3,
            const char* format,
            ...)
{
    va_list argList;
    va_start(argList, format);

#define BUF_SIZE 500
    char posMsg[BUF_SIZE] = { 0 };
    char errMsg[BUF_SIZE] = { 0 };
    char userMsg[BUF_SIZE] = { 0 };
    snprintf(posMsg, BUF_SIZE, "[%s][%s:%d]", 
             details::kLogLevelName[level], fileName, lineNumber);

    if(level >= LogLevel::ERROR)
    {
        snprintf(errMsg, BUF_SIZE, "[%s: %s]",
                (savedErrno > 0 && savedErrno < MAX_ENAME) ?
                ename[savedErrno] : "UNKNOWN",
                strerror(savedErrno));
    }
    vsnprintf(userMsg, BUF_SIZE, format, argList);
    va_end(argList);

#undef BUF_SIZE

    if(level >= LogLevel::ERROR)
    {  
        fprintf(stderr, "%s%s%s\n", posMsg, errMsg, userMsg);
    }
    else
    {    
        fprintf(stderr, "%s%s\n", posMsg, userMsg);
    }
    if(flushStdout) fflush(stdout);
    fflush(stderr);

    if(level >= LogLevel::FATAL)
    {
        details::Terminate(useExit3);
    }
}

class ClockTime
{
public:
    void start()
    {
        m_startTime = clock();
    }

    void stop()
    {
        m_stopTime = clock();
    }

    double duration()
    {
        return static_cast<double>(m_stopTime - m_startTime) / (CLOCKS_PER_SEC);
    }   

private:
    clock_t m_startTime;
    clock_t m_stopTime;
};

/// TODO: 确认 chrono 使用
class WallTime
{
public:
    void start()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    void stop()
    {
        m_stopTime = std::chrono::high_resolution_clock::now();
    }

    double duration()
    {
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(m_stopTime - m_startTime);
        return static_cast<double>(nanoseconds.count()) / kNanosecondsPerSecond;
    }

private:
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_stopTime;
    static const int kNanosecondsPerSecond = 1000 * 1000 * 1000;
};

bool CreateFile(const std::string& fileName, int64_t fileSize, int lineSize)
{
    if(fileSize <= 0) 
    {
        LOG_ERROR("File size must > 0, file size = %ld\n", fileSize);
        return false;
    }
    
    mode_t old = umask(0022);
#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
    int fd = open(fileName.c_str(), O_WRONLY | O_TRUNC | O_CREAT, RWRWRW);
    if(-1 == fd)
    {
        LOG_ERROR("file open error, path = %s!", fileName.c_str());
        return false;
    }
#undef RWRWRW
    umask(old);

    /// ' ' 与 '~' 分别是 ASCII 编码中最小和最大的可见字符 
    const int mod = int('~' - ' ') + 1;
    int64_t writtenFileSize = 0;
    char buf[BUFSIZ];
    srand(static_cast<unsigned int>(time(nullptr)));
    while(writtenFileSize < fileSize)
    {
        if(writtenFileSize % 10000 == 0) 
        {
            srand(static_cast<unsigned int>(time(nullptr)));
        }

        int bufSize = (fileSize - writtenFileSize > BUFSIZ ? 
            BUFSIZ :  static_cast<int>(fileSize - writtenFileSize));

        int writtenBufSize = 0;

        while(writtenBufSize < bufSize)
        {
            if((writtenFileSize + writtenBufSize) % lineSize == 0)
                buf[writtenBufSize++] = '\n';
            else
                buf[writtenBufSize++] = static_cast<char>(rand() % mod + ' ');  
        }
        int64_t len = write(fd, buf, writtenBufSize);
        assert(len == writtenBufSize);
        writtenFileSize += len;
    }
    
    return true;
}

typedef struct interaction {
	int id;
	char name[8];
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} Interaction;

/// 进程间同步代码参考 https://blog.csdn.net/u011583798/article/details/104382550
/// TODO: to learn
Interaction* GetInteraction(const char *filename)
{
    Interaction *share_mem;
	int len = sizeof(Interaction);

    int fd = shm_open(filename, O_RDWR|O_CREAT|O_EXCL, 0777);
    if (fd > 0) {
        /**< 设置共享内存长度 */
        ftruncate(fd, len);
        share_mem = (Interaction *)mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

        pthread_condattr_t cond_attr;
        pthread_condattr_init(&cond_attr);
        pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&share_mem->cond, &cond_attr);


        pthread_mutexattr_t mutex_attr;
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&share_mem->mutex, &mutex_attr);

		// printf("mmap Interaction object(%p) ok\n", share_mem);
    } else {
		// perror("ready");
        fd = shm_open(filename, O_RDWR, 0777);
        /**< 其他进程已经初始化过了，这里不要再初始化 */
        share_mem = (Interaction *)mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		// printf("get Interaction object(%p) ok\n", share_mem);
    }
    close(fd);
//    shm_unlink("/test_share_mem"); 

    return share_mem;
}

void Wait(Interaction * share_mem)
{
    pthread_mutex_lock(&share_mem->mutex);
    pthread_cond_wait(&share_mem->cond, &share_mem->mutex);
    pthread_mutex_unlock(&share_mem->mutex);
}

void Singal(Interaction * share_mem)
{
    pthread_cond_signal(&share_mem->cond);
}

/** 
 * Return a string containing the current time formatted according to
 * the specification in 'format' (see strftime(3) for specifiers).
 * If 'format' is NULL, we use "%c" as a specifier (which gives the'
 * date and time as for ctime(3), but without the trailing newline).
 * Returns NULL on error. 
 * 
 * nonreentrant
 */
char* CurrTime(const char *format)
{
    /// TODO: 实现 CurrTime_r
#define BUF_SIZE 1000

    /// 不可重入（多线程调用或在中断处理函数中重复调用会导致 buf 覆盖，返回错误时间）
    static char buf[BUF_SIZE];  /* Nonreentrant */

    time_t t;
    size_t s;
    struct tm *tm;

    t = time(NULL);
    tm = localtime(&t);
    if (tm == NULL)
        return NULL;

    s = strftime(buf, BUF_SIZE, (format != NULL) ? format : "%c", tm);
#undef BUF_SIZE
    return (s == 0) ? NULL : buf;
}



#endif // __HEADER_H__