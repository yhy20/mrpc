#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    /**
     * perror(3) man 3 perror
     * 
     * #inlcude <stdio.h>
     * void perror(cosnt char*s);
     * 
     * #include <errno.h>
     * const char *sys_errlist[];
     * int sys_nerr;
     * int errno;
     * 
     * The routine perror() produces a message on the standard error output,
     * describing the last error encountered during a call to a system or
     * library  function. First (if s is not NULL and *s is not a null byte
     * ('\0')) the argument string s is printed, followed by a colon and a
     * blank. Then the message and a new-line.
     * 
     * To be of most use, the argument string should include the name of the
     * function that incurred the error. The error number is taken from the
     * external variable errno, which is set when errors occur but not cleared
     * when successful calls are made.（要使用 errno，需 #include <errno.h>）
     */
    int fd = 100;
    int one = 1;
    ssize_t len = write(fd, &one, sizeof(one));
    if(-1 == len) 
    {
        /// use perro to print error.
        /**
         * When a system call fails, it usually returns -1 and
         * sets the variable errno to a value describing what
         * went wrong. (These values can be found in <errno.h>.)
         * Many library functions do likewise. The function per‐
         * ror()  serves to translate this error code into human-
         * readable form. Note that errno is undefined after a
         * successful library call: this call may well change
         * this variable, even though it succeeds, for example
         * because it internally used some other library function
         * that failed. Thus, if a failing call is not immedi‐
         * ately followed by a call to perror(), the value of
         * errno should be saved.
         * 
         * (1) Many library functions do likewise.
         *     许多库函数（内部不一定调用了系统调用）也像系统调用一样
         *     设置 errno 来表示错误，以通知调用者处理错误
         * (2) Note that errno is undefined after a successful library call
         *     系统调用或库函数调用成功也可能会更改 errno 的值。绝大多数实现在
         *     调用成功后不会更改 errno 的值，errno 只用于获取错误信息，是否发生
         *     错误必须要依靠调用返回值来判断（通常错误发生时返回 -1）
         */
        perror("write");

        /**
         * After #include <errno.h>
         * The global error list sys_errlist[] indexed by errno
         * can be used to obtain the error message without the
         * newline. The largest message number provided in the
         * table is sys_nerr - 1. Be careful when directly access‐
         * ing this list because new error values may not have
         * been added to sys_errlist[].The use of sys_errlist[]
         * is nowadays deprecated.
         * 
         * The following code is equivalent to using perror("write")
         */
        fprintf(stderr, "write: %s\n", sys_errlist[errno]);

        exit(EXIT_FAILURE);
    }
    
    return 0;
}