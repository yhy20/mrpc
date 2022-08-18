#include "../../header.h"

int main()
{   
    const int fileSize = 1000 * 1024 * 1024;
    const int lineSize = 80;

    mode_t old =  umask(0022);
#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
    int fd = open("./data.txt", O_WRONLY | O_TRUNC | O_CREAT, RWRWRW);
    if(-1 == fd) LOG_ERROR("open() error!");
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
    
    exit(EXIT_SUCCESS);
}