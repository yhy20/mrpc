#include "../header.h"

#define BUF_SIZE 3

int main(int argc, char* argv[])
{
    char buf[BUF_SIZE];
    if(-1 == access("./data.txt", F_OK))
    {
        CreateFile("./data.txt", 300 * 1024 * 1024, 100);
    }
    
    FILE* fp1 = fopen("./data.txt", "r");
    if(nullptr == fp1) LOG_FATAL("open() error!");
    FILE* fp2 = fopen("./data.copy.txt", "w");
    if(nullptr == fp2) LOG_FATAL("open() error!");
    
    ssize_t len = 0;
    while((fgets(buf, BUF_SIZE, fp1)) != nullptr)
        fputs(buf, fp2);

    fclose(fp1);
    fclose(fp2);
    exit(EXIT_SUCCESS);
}