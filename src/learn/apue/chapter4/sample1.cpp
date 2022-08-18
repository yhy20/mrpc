#include <sys/stat.h>
#include <libgen.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>

// 非线程安全
#define LOG_ERROR(msg) printf("[%s:%d] %s: %s\n", basename((char*)__FILE__), __LINE__, msg, strerror(errno));  

void test1()
{
    // #include <sys/stat.h>
    // int stat(const char* restrict pathname, struct stat* restrict buf);
    struct stat buf1, buf2, buf3, buf4;
    if(-1 == stat("./dir/poetry.txt", &buf1))
        LOG_ERROR("stat() error!");
    if(-1 == fstat(STDIN_FILENO, &buf2))
        LOG_ERROR("fstat() error!");
    if(-1 == lstat("/dev/fd", &buf3))
        LOG_ERROR("lstat() error!");
    int fd = open("/dev", O_DIRECTORY | O_RDONLY);
    if(-1 == fd) LOG_ERROR("open() error!");
    if(-1 == fstatat(fd, "fd", &buf4, AT_SYMLINK_NOFOLLOW))
        LOG_ERROR("fstatat() error!");

    // stat 结构体包含的信息 
    // mode_t           st_mode;    /* file type & mode (permissions) */  
    // ino_t            st_ino;     /* i-node number (serial number) */
    // dev_t            st_dev;     /* device number (file system) */
    // nlink_t          st_nlink;   /* number of links  */
    // uid_t            st_uid;     /* user ID of owner */
    // gid_t            st_gid;     /* group ID of owner */
    // off_t            st_size;    /* size in bytes, for regular files */
    // struct timespec  st_atime;   /* time of last access */
    // struct timespec  st_mtime;   /* time of last modification */
    // struct timespec  st_ctime;   /* time of last file status change */
    // blksize_t        st_blksize; /* best I/O block size */
    // blkcnt_t         st_blocks;  /* number of disk blocks allocated */
    // POSIX.1 未要求 st_rdev, st_blksize 和 st_blocks 字段
    // Single UNIX Specification XSI 扩展定义了这些字段

    // timespec 结构类型按照秒和纳秒定义了时间，至少包括下面两个字段
    // time_t   tv_sec;
    // long     tv_nsec;

    // 在 2008 年版以前的标志中，时间字段定义成 st_atime, st_mtime, st_ctime
    // 当前版本中定义成了 st_atim, st_mtim, st_ctim，为了保持兼容性，头文件中有如下 #define 语句
    // # define st_atime st_atim.tv_sec	/* Backward compatibility */
    // # define st_mtime st_mtim.tv_sec
    // # define st_ctime st_ctim.tv_sec
}   
 
void test2()
{   
    // #include <sys/stat.h>
    // mode_t umask(mode_t cmask);
    
    // #define	__S_ISUID	04000	/* Set user ID on execution.  */
    // #define	__S_ISGID	02000	/* Set group ID on execution.  */
    // #define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */

    // #define	__S_IREAD	0400	/* Read by owner.  */
    // #define	__S_IWRITE	0200	/* Write by owner.  */
    // #define	__S_IEXEC	0100	/* Execute by owner.  */
    // #define  S_IRUSR	    __S_IREAD       /* Read by owner.  */
    // #define  S_IWUSR	    __S_IWRITE      /* Write by owner.  */
    // #define  S_IXUSR	    __S_IEXEC       /* Execute by owner.  */
    // # define S_IRUSR	__S_IREAD       /* Read by owner.  */
    // # define S_IWUSR	__S_IWRITE      /* Write by owner.  */
    // # define S_IXUSR	__S_IEXEC       /* Execute by owner.  */
    // /* Read, write, and execute by owner.  */
    // # define S_IRWXU	(__S_IREAD|__S_IWRITE|__S_IEXEC)

    // # define S_IRGRP	(S_IRUSR >> 3)  /* Read by group.  */
    // # define S_IWGRP	(S_IWUSR >> 3)  /* Write by group.  */
    // # define S_IXGRP	(S_IXUSR >> 3)  /* Execute by group.  */
    // /* Read, write, and execute by group.  */
    // # define S_IRWXG	(S_IRWXU >> 3)

    // # define S_IROTH	(S_IRGRP >> 3)  /* Read by others.  */
    // # define S_IWOTH	(S_IWGRP >> 3)  /* Write by others.  */
    // # define S_IXOTH	(S_IXGRP >> 3)  /* Execute by others.  */
    /* Read, write, and execute by others.  */
    // # define S_IRWXO	(S_IRWXG >> 3)

    #define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

    mode_t oldMode = umask(0);
    if(-1 == creat("./dir/foo", RWRWRW)) LOG_ERROR("creat() error");
    umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if(-1 == creat("./dir/bar", RWRWRW)) LOG_ERROR("creat() error");
    umask(oldMode);

    #undef RWRWRW

    std::cout << std::oct << S_IRWXU << std::endl;
    std::cout << std::oct << (S_IRWXU | S_IRWXG) << std::endl;
    std::cout << std::oct << (S_IRWXU | S_IRWXG | S_IRWXO) << std::endl;
}

// 函数 chmode、fchmod 与 fchmodat
void test3()
{   
    // #include <sys/stat.h>
    // int chmod(const char* pathname, mode_t mode);
    // int fchmod(int fd, mode_t mode);
    // int fchmodat(int fd, const char *pathname, mode_t mode, int flag);
    // 为了改变一个文件的权限位，进程的有效用户 ID 必须等于用户的所有者 ID，或者该进程必须具有超级用户权限
    // 参数 mode 是以下常量的按位或
    
    struct stat statbuf;
    if(-1 == stat("./dir/foo", &statbuf)) LOG_ERROR("stat() error");
    if(-1 == chmod("./dir/foo", ((statbuf.st_mode & ~S_IXGRP) | S_ISUID))) LOG_ERROR("chmod() error");
    if(-1 == chmod("./dir/bar", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) LOG_ERROR("chmod() error");
}

void test_errno()
{
    // Linux中系统调用的错误都存储于 errno 中，头文件 errno.h，范围为 0-255
    // errno 只保存最近一次的系统错误（一般用在调用系统函数时），下一次的错误码会覆盖掉上一次的错误。
    // 在单线程的程序中，errno 是全局变量，在多线程序中，每个线程有各自的 error，是线程安全的
    /* When using threads, errno is a per-thread value */

    // 有三个函数可以打印错误信息分别如下：
    // stdio.h 中：void perror(const char *s) 函数本身会打印字符串 s 的信息加上冒号和空格，再打印 error 相关信息
    // string.h 中：char *strerror(int errnum) 参数 errno，返回错误描述字符串，不是线程安全的
    // string.h 中：char *strerror_r(int errnum, char *buf, size_t buflen) 返回错误描述字符串，是线程安全的
    int fd = open("none.txt", O_RDONLY);
    if(-1 == fd) 
    {
        perror("open() error");
        printf("open() error: %s\n", strerror(errno));
        LOG_ERROR("open() error");
    }
}

int main()
{
    // test1();
    // test2();
    test3();
    // test_errno();
    return 0;
}