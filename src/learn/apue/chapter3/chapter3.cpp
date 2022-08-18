#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

namespace 
{

char msg[] = "Hello, World!\n";
char buf[1024] = {0};

void errorMsg(const char* msg)
{
    std::cerr << msg << std::endl;
}

}

void test1()
{

    // #include <fcntl.h>
    // int open(const char* path, int oflag, ...);
    // path : 文件位置的相对路径或绝对路径
    // ... : 可变参数列表，仅当创建新文件时，才使用这个参数 
    // oflag: 打开文件的多个选项
    // O_RDONLY O_WRONLY O_RDWR 
    // O_EXEC 只执行打开
    // O_SEARCH 只搜索打开（应用于目录，目前的系统尚未实现）
    // 以上的 5 个常量必须指定一个，且只能指定一个

    // 下列是可选常量
    // O_APPEND
    // O_CLOEXEC 把 FD_CLOEXEC 常量设置为文件描述符标志
    // O_CREAT 若文件不存在则创建，应用此选项时，open 函数需同时说明第 3 个参数 mode
    // openat 函数需同时说明第 4 个参数 mode, 用 mode 指定该文件的访问权限位
    // O_DIRECTORY
    // O_EXCL 
    // O_NOCTTY 如果 path 引用的是终端设备，则不将该设备分配作为此进程的控制终端
    // O_NOFOLLOW 如果 path 引用的是一个符号链接，则出错
    // O_NONBLOCK
    // O_SUNC
    // O_TRUNC
    // O_TTY_INIT
    // O_DSYNC
    // O_RSYNC

    // (1) open 和 openat 函数返回的文件描述符一定是最小未使用的文件描述符号
    // 这一点被某些应用程序用来在标志输入，标准输出或标准错误上打开新文件
    int c = read(STDIN_FILENO, buf, 10);
    // 读取的时候包含了换行符
    printf("c = %d\n", c);
    write(STDOUT_FILENO, buf, c);
    write(STDERR_FILENO, buf, c);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // 所有打印都被重定向到了文件内部
    int infd = open("./dir/in.txt", O_RDONLY);
    int outfd = open("./dir/out.txt", O_WRONLY);
    int errfd = open("./dir/error.txt", O_WRONLY);

    // 遗留问题，使用 dup2 函数保证在一个给定的文件描述符上打开文件

    // (2) openat 函数
    // a) 若传递给 path 指定的是绝对路径名，则 fd 参数将被忽略
    // 此时 openat 函数就相当于 open 函数
    int fd1 = openat(AT_FDCWD, "/root/c++_language/unix/dir/file1.txt", O_RDWR | O_TRUNC);
    if(-1 == fd1) errorMsg("openat() error!");
    if(-1 == write(fd1, msg, sizeof(msg) - 1));

    // 重新设置读取时候的偏移量
    off_t offset = lseek(fd1, 0, SEEK_SET);
    if(-1 == offset) errorMsg("lseek() error!");

    // 读取文件中的信息
    memset(buf, 0, sizeof(buf));
    int readSize = read(fd1, buf, sizeof(msg) - 1);
    if(-1 == readSize) errorMsg("read() error!");
    buf[readSize] = '\0';
    printf("read size = %d, buf = %s", readSize, buf);

    // b) 若传递给 path 指定的是相对路径名，则 fd 参数应该传入一个目录的文件句柄
    // 它表示相对路径名所指文件所在的具体目录，此时 openat 可以打开特定目录下的文件
    int dirFd = open("./dir", O_RDONLY | O_DIRECTORY);
    int fd2 = openat(dirFd, "file2.txt", O_WRONLY | O_TRUNC);
    if(-1 == dirFd) errorMsg("open() !");
    if(-1 == fd2) errorMsg("openat() error!");
    // PS1：如果用 const char* str = "Hello, World!\n";
    // 则 sizeof(str) 输出的是指针的大小，不是字符串本身的大小
    // PS2：sizeof 算出的大小包括了字符串最后 null 字符，注意 -1
    std::cout << sizeof(msg) << std::endl;
    if(-1 == write(fd2, msg, sizeof(msg) - 1))
        errorMsg("write() error!");

    // c) 若传递给 path 指定的是相对路径名，fd 参数具有特殊值 AT_FDCWD
    // 这种情况下表示从当前工作目录打开指定的文件
    int fd3 = openat(AT_FDCWD, "dir/file3.txt", O_WRONLY | O_TRUNC);
    if(-1 == fd3) errorMsg("openat() error!");
    if(-1 == write(fd3, msg, sizeof(msg) - 1))
        errorMsg("write() errlr!");

    close(infd);
    close(outfd);
    close(errfd);
}

void test2()
{
    // #include <fcntl.h>
    // int creat(const char* path, mode_t mode); 此函数等效于
    // open(path, O_WRONLY | O_CREAT | O_TRUNC, mode)
    // 在早期的 UNIX 系统版本中，open 的第二个参数只能是 0、1 或 2，
    // 即只能以 O_RDONLY, O_WRONLY 或 ORDWR 方式打开一个文件，且无法打开一个尚未
    // 存在的文件，因此需要另一个系统调用 creat 以创建文件。现在 open 函数提供了
    // O_CREAT 和 O_TRUNC，也就不再需要单独的 creat 函数了。

    // create 的一个不足是只能以 O_WRONLY 方式打开所创建的文件，在提供新版本的
    // open 函数前，要以读写模式创建一个文件，则需先 close 创建的文件，然后再调用 open 重新打开文件
    // open(path, O_RDWR | O_CREAT | O_TRUNC, mode);

    // #include <unistd.h>
    // int close(int fd);
    // 关闭一个文件时还会释放该进程加在该文件上的所有记录锁
    // 当一个进程终止时，内核自动关闭它所有打开的文件，很多程序利用这一点不显式地用 close 关闭打开的文件    
}

void test3()
{

}

void test4()
{
    // #include <unistd.h>
    // int dup(int fd);
    // 用于复制一个现有的文件描述符，由 dup 返回的文件描述符一定是当前可用文件描述符中的最小值
    // dup(fd) 等效于 fcntl(fd, F_DUPFD, 0);
    int fd1 = dup(STDIN_FILENO);
    write(fd1, msg, sizeof(msg) - 1);
    // 遗留问题， 查看 close-on-exec 标志

    // int dup2(int fd, fd2);
    // 对于 dup2，可以用 fd2 参数指定新描述符的值，如果 fd2 已经打开，则先将其关闭
    // 若 fd 等于 fd2，则 dup2 返回 fd2 而不关闭它
    // dup2(fd, fd2) 等效于 close(fd2); dup2(fd, fd2);
    int fd2 = dup2(STDIN_FILENO, 100);
    write(100, msg, sizeof(msg) - 1);
    
    // PS: dup 函数会清除 close-on-exec 标志
    close (fd2);
}

void test5()
{
    // 延迟写，内核缓冲区高速缓存或页高速缓存
    // 当向文件写数据时，内核通常先将数据复制到缓冲区，然后排入队列，晚些时候再写入磁盘
    // 通常，当内核需要使用缓冲区来存放其他磁盘数据块时，会把所有延迟写数据写入磁盘
    // 为了保证磁盘上实际文件系统与缓冲区中内容的一致性，UNIX 系统提供了 sync, fsync 和 fdatasync 三个函数
    // #include <unistd.h>
    // void sync(void);
    // int fsync(int fd);
    // int fdatasync(int fd);

    // sync 只是将所修改过的块缓冲区排入写队列，然后就返回，它并不等待实际写磁盘操作结束
    // fsync 函数只对由文件描述符 fd 指定的一个文件起作用，并且等待写磁盘操作结束才返回
    // fdatasync 函数类似于 fsync，但它只影响文件的数据部分。而除数据外，fsync 还会同步更新文件的属性

}

void test6()
{
    // #include <fcntl.h>
    // int fcntl(int fd, int cmd, ...);
    // fcntl 用于改变已打开文件的属性
    // (1) F_DUPFD 复制一个已有的文件描述符，并将新文件描述符作为函数返回值，这个新文件描述符满足下列条件
    // a) 新文件描述符的值为大于或等于 args 参数的尚未打开的最小值
    // b) 新描述符与 fd 共享一个文件表项，但是新的文件描述符有它自己的一套文件描述符标志
    // c) 新文件描述符的 FD_CLOEXEC 文件描述符标志被清除，这表示该文件描述符在 exec 时仍有效
    // (2) F_DUPFD_CLOEXEC
    // 与 F_DUPFD 功能一致，唯一的不同是会设置 CLOSE_ON_EXEC 标志
    int afd1 = open("/dev/stdout", O_WRONLY);
    int afd2 = open("/dev/stdout", O_WRONLY | O_CLOEXEC);
    int av1 = fcntl(afd1, F_GETFD);
    int av2 = fcntl(afd2, F_GETFD);
    printf("After call fcntl(fd, F_GETFD), av1 = %d, av2 = %d\n", av1, av2);

    int afd3 = fcntl(afd1, F_DUPFD, 100);
    int afd4 = fcntl(afd2, F_DUPFD, 100);
    int av3 = fcntl(afd3, F_GETFD);
    int av4 = fcntl(afd4, F_GETFD);
    // 可以看到 afd2 的 FD_CLOEXEC 标志被清零
    printf("After call fcntl(fd, F_DUPFD), av3 = %d, av4 = %d\n", av3, av4);

    int afd5 = fcntl(afd1, F_DUPFD_CLOEXEC, 100);
    int afd6 = fcntl(afd2, F_DUPFD_CLOEXEC, 100);
    int av5 = fcntl(afd5, F_GETFD);
    int av6 = fcntl(afd6, F_GETFD);
    // 可以看到 afd1 的 FD_CLOEXEC 标志设置
    printf("After call fcntl(fd, F_DUPFD_CLOEXEC), av5 = %d, av6 = %d\n", av5, av6);

    // (3) F_GETFD 返回 fd 的文件描述符标志的值，当前系统只定义了一个文件描述符标志 FD_CLOEXEC
    // (4) F_SETFD 将文件描述符标志 close_on_exec 标志设置为第三个参数传递的值
    fcntl(afd1, F_SETFD, FD_CLOEXEC);  //fcntl(afd1, F_SETFD, 1); 
    fcntl(afd2, F_SETFD, 0);
    av1 = fcntl(afd1, F_GETFD);
    av2 = fcntl(afd2, F_GETFD);
    printf("After call fcntl(fd, F_SETFD), av1 = %d, av2 = %d\n", av1, av2);

    // (5) F_GETFL 将对应选项的文件状态标志作为函数值返回
    // int bfd1 = open("/dev/stdout", F_GETFL, )


    // (6) F_SETFL

    // (7) F_GETOWN 获取档期接收 SIGIO 和 SIGURG 信号的进程 ID 或进程组 ID

    // (8) F_SETOWN 设置接收 SIGIO 和 SIGURG 信号的进程 ID 或进程组 ID
    // 正的 arg 指定一个进程 ID，负的 arg 表示等于 arg 绝对值的一个进程组 ID

    // fcntl 的返回值 与命令有关， 如果出错，所有命令都返回 -1
    // F_DUPFD 和 F_FDUPFD_CLOEXEC f返回新的文件描述符
    // F_GETFD 和 F_GETFL 返回对应标志的值
    // F_GETOWN 返回正的进程组 ID 或 负的进程组 ID

}

void test7()
{
    // /dev/fd -> /proc/self/fd 的目录项是名为 0、1、2 等的文件
    // 打开文件 /dev/fd/n 等效于复制描述符（假定描述符 n 是打开的）
    // fd = open("dev/fd/0, mode"); 等效于 fd = dup(0);
    // 所以描述符 0 和 描述符 fd 共享同一文件表项，例如描述符 0 先前被只读
    // 打开，那么我们也只能对 fd 进行读操作。

}

int main()
{
    // test1();
    // test4();
    test6();
}