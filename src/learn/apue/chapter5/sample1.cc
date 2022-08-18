#include <stdio.h>
#include <wchar.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

            
#include "apue.h"
#include <bitset>  

// 一、流的定向（stream's orientation)
void test1()
{
    // 对于 ASCII 字符集，一个字符由一个字节表示
    // 对于国际字符集（一般兼容 ASCII 字符集），一个字符由多个字节表示
    // 标志 I/O 文件流可用于单字节或多字节字符集
    // 流的定向决定了 I/O 读写所使用的字符集
    // 当一个流最初被创建时，它是没有定向的。
    // 如若在未
    
    // int fwide(FILE * fp, int mode);
    // FILE* fp = fopen("./dir/file.txt", "w+");
    // char buf[BUFSIZ];
    // setbuf(fp, buf)
    // 相当于调用了 setbuf(fp, buf, buf ? _IOFBF : _IONBF, BUFSIZE);
    //fwide(fp, -1);
    // char buf[2014] = {0};
    // fgets(buf, sizeof(buf), stdin);
    // printf("%s\n", buf);


    // int c = getchar();

    // if(-1 == c) 
    // {   
        
    //     std::cout << std::hex << c << std::endl;
    //     std::cout << "二进制： "<< std::bitset<sizeof(c)*8>(c) << std::endl;
    //     if(feof(stdin)) std::cout << "At the end of stdin!" << std::endl;
    //     if(ferror(stdin)) std::cout << "Error of stdin!" << std::endl;
    //     // LOG_ERROR("getchar() error!");
    // }
    // std::cout << c << std::endl;

    // c = getchar();
    // if(-1 == c)
    // {

    // }
    // std::cout << (unsigned char)c << std::endl;

    // 证明行缓冲
    fputs("Hello", stdout);
    sleep(3);
    putc('\n', stdout);
    sleep(3);
    // fflush(stdout);

}

// 二进制 I/O
void test2()
{
    // 之前是基于字符的 I/O 操作
    // #include <stdio.h>
    // size_t fread(void *restrict ptr, size_t size, size_t nobj, FILE *restrict fp);
    // size_t fwrite(const void *restrict ptr, size_t size, size_t nobj, FILE *restrict fp)
    float data[10];
    
}

// 
test3()
{

}

int main()
{
    // test1();
    test2();
    return 0;
}