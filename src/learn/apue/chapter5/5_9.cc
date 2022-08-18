#include "apue.h"
#include <vector>
#include <string>
#include <fstream>
#define NAME_SIZE 10

struct Item
{
    short count = 0;
    long total = 0;
    char name[NAME_SIZE] = {0};
};


void test1()
{
    Item from;
    from.count = SHRT_MAX;
    from.total = LONG_MAX;
    strcpy(from.name, "Tim");
    printf("sizeof(Item) = %ld\n", sizeof(Item));

    FILE* fp = fopen("./dir/out1.bin", "w+");
    if(NULL == fp) LOG_ERROR("fopen() error");
    if(1 != fwrite(&from, sizeof(from), 1, fp))
        LOG_ERROR("fwrite() error");

    long offset = ftell(fp);
    printf("offset = %ld\n", offset);

    if(-1 == fseek(fp, 0, SEEK_SET))
        LOG_ERROR("fseek() error");

    offset = ftell(fp);
    printf("offset = %ld\n", offset);
    
    Item to;
    // void *buf = malloc(sizeof(Item));
    if(1 != fread(&to, sizeof(Item), 1, fp))
    {
        if(ferror(fp)) printf("Read error!\n");
        if(feof(fp)) printf("At end of file!\n");
        LOG_ERROR("fread() error");
    }
    // Item* to = (Item*)buf;
    // printf("to.count = %d\n", to->count);
    // printf("to.total = %ld\n", to->total);
    // printf("to.name = %s\n", to->name);
    printf("to.count = %d\n", to.count);
    printf("to.total = %ld\n", to.total);
    printf("to.name = %s\n", to.name);
}

void test2()
{
    std::string from = "Hello, World!";
    FILE* fp = fopen("./dir/out2.bin", "w+");
    if(NULL == fp) LOG_ERROR("fopen error!");
    int len = sizeof(from);
    printf("sizeof(from) = %d\n", len);
    if(1 != fwrite(&from, len, 1, fp))
    {
        if(ferror(fp)) printf("error!\n");
        if(feof(fp)) printf("EOF\n");
        LOG_ERROR("fwrite() error!");
    }
    
    if(-1 == fseek(fp, 0, SEEK_SET))
        LOG_ERROR("fseek() error!");

    void* buf = malloc(len);
    if(1 != fread(buf, len, 1, fp))
        LOG_ERROR("fread() error!");
    
    std::string* to = (std::string*)buf;
    std::cout << *to << std::endl;

    printf("from.data() = %p\n", from.data());
    printf("to.data() = %p\n", to->data());

    std::string from1 = "Hello, World!";
    std::string to1 = from1;

    // COW 写时拷贝
    printf("from1.data() = %p\n", from1.data());
    printf("to1.data() = %p\n", to1.data());

    int size = sizeof(from1);
    unsigned char* p1 = (unsigned char*)&from1;
    unsigned char* p2 = (unsigned char*)&to;
    std::cout << size << std::endl;

    // TODO: 如何打印单个字节查看内存
    // 查看内存信息
    for(int i = 0; i < size; ++i)
    {
        printf("%.2x", p1[i]);
    }
    std::cout << std::endl;
    for(int i = 0; i < size; ++i)
    {
        printf("%.2x", p2[i]);
    }
    std::cout << std::endl;
}


void test3()
{
    //std::vector<std::string> v = {""}
}

// 定位流
void test4()
{
    // unix 系统并不区分文本文件和二级制文件
    unsigned char num_str[] = { '1', '2', '3', '4'};
    unsigned char num[] = { 0x04, 0x12 };
    unsigned char html_str[] = { 'h', 't', 'm', 'l'};
    unsigned char html[] = { 0x68, 0x74, 0x6D, 0x6C };

    FILE* fp_txt = fopen("./dir/out4.txt", "w");
    FILE* fp_bin = fopen("./dir/out4.bin", "w");
    if(NULL == fp_txt || NULL == fp_bin) LOG_ERROR("fopen error!");

    fwrite(num_str, sizeof(unsigned char), sizeof(num_str), fp_txt);
    putc('\n', fp_txt);
    fwrite(html_str, sizeof(unsigned char), sizeof(html_str), fp_txt);
    
    fwrite(num, sizeof(unsigned char), sizeof(num), fp_bin);
    putc('\n', fp_bin);
    fwrite(html, sizeof(unsigned char), sizeof(html), fp_bin);

}

void test5()
{
    // unix 下无论无论是否为 binary 模式，C++ fstream 都将数据转化成文本后再输出
    std::fstream os_txt("./dir/out5.txt", std::ios::out | std::ios::trunc);
    std::fstream os_bin("./dir/out5.bin", std::ios::out | std::ios::trunc | std::ios::binary);
    const char* num_str = "1234";
    int num = 1234;
    os_txt << num << num_str;
    os_bin << num << num_str;

    os_txt.close();
    os_bin.close();

    // 超出的数值会截断
    std::fstream os("./dir/in5.txt", std::ios::in);
    int a = 0;
    int b = 0;
    double c = 0;
    os >> a >> b >> c;
    printf("a=%d\nb=%d\nc=%f\n", a, b, c);
    
    // 截断导致的问题
    if(os.eof()) std::cout << "EOF" << std::endl;
}

int main()
{   
    // test2();
    // test3();
    // test4();
    test5();
    return 0;
}