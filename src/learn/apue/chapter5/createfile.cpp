#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <iostream>

// 100 * 1000 * 1000
#define FILE_SIZE 100000000

int main()
{
    mode_t old =  umask(0022);
#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
    int fd = open("./dir/data.txt", O_WRONLY | O_TRUNC | O_CREAT, RWRWRW);
#undef RWRWRW
    umask(old);

    // ' ' 与 '~' 分别是 ASCII 编码中最小和最大的可见字符 
    int mod = int('~' - ' ') + 1;
    int file_size = 0;
    char buf[BUFSIZ];
    srand((unsigned int)time(NULL));
    while(file_size < FILE_SIZE)
    {
        if(file_size % 10000 == 0) 
        {
            srand((unsigned int)time(NULL));
        }
        int block_size = 0;
        while(block_size < BUFSIZ && file_size + block_size < FILE_SIZE)
        {
            if((file_size + block_size) % 100 == 0)
                buf[block_size++] = '\n';
            else
                buf[block_size++] = rand() % mod + int(' ');  
        }
        int write_size = write(fd, buf, block_size);
        assert(write_size == block_size);
        file_size += block_size;
    }
    
    return 0;
}