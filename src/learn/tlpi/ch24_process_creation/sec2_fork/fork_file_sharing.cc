
#include "../../header.h"

int main(void)
{
    int fd, flags;
    char temp[] = "/tmp/testXXXXXX";

    setbuf(stdout, nullptr);
    fd = mkstemp(temp);
    if(-1 == fd) LOG_FATAL("mkstemp() error!");

    LOG_INFO("File offset before fork(): %lld",
            (long long) lseek(fd, 0, SEEK_CUR));
    
    flags = fcntl(fd, F_GETFL);
    if(-1 == flags) LOG_FATAL("fnctt(fd, F_GETFL) error!");
    LOG_INFO("O_APPEND flag before fork() is: %s",
            (flags & O_APPEND) ? "on" : "off");

    switch(fork())
    {
    case -1:
        LOG_FATAL("fork() error!");
    case 0: // child: change file offset and status flags.
        if(-1 == lseek(fd, 1000, SEEK_SET))
            LOG_FATAL("lseek() error!");
        
        flags = fcntl(fd, F_GETFL);
        if(-1 == flags) LOG_FATAL("fcntl(fd, F_GETFL) error!");
        flags |= O_APPEND;
        if(-1 == fcntl(fd, F_SETFL, flags))
            LOG_FATAL("fcntl(fd, F_SETFL, flags error!");
        _exit(EXIT_SUCCESS);

    default: // parent: can see file changes made by child.
        if(-1 == wait(nullptr)) LOG_FATAL("wait() error!");
        LOG_INFO("Child has exited!");
        LOG_INFO("File offset in parent: %lld",
                (long long)lseek(fd, 0, SEEK_CUR));

        flags = fcntl(fd, F_GETFL);
        if(-1 == flags) LOG_FATAL("fcntl(fd, F_GETFL) error!");
        LOG_INFO("O_APPEND flag in parent is: %s",
                (flags & O_APPEND) ? "on" : "off");
        exit(EXIT_SUCCESS);
    }
}