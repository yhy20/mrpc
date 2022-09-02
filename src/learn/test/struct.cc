#include <stdint.h>
#include <iostream>

/**
 * 内存对齐问题
 * 结构体内成员按照声明顺序存储，第一个成员地址和整个结构体地址相同。
 * 未特殊说明时，按结构体中 size 最大的成员对齐（若有 double 成员，按 8 字节对齐）
 * C++11 后引入了两个关键字 alignas 与 alignof，其中 alignas 可以指定结构体的对齐
 * 方式，alignof 可以计算出类型的对齐方式。
 * 
 * 内存对齐的作用
 * 
 */

struct Info 
{
    uint8_t a;
    uint16_t b;
    uint8_t c;
};

struct alignas(4) Info2
{
    uint8_t a;
    uint16_t b;
    uint8_t c;
};

/// alignas 失效情况，若 alignas 小于自然对齐的最小单位，则被忽略。
struct alignas(2) Info3
{
    uint8_t a;
    uint32_t b;
    uint8_t c;
};

int main(void)
{
    std::cout << sizeof(Info) << std::endl;
    std::cout << alignof(Info) << std::endl;
    std::cout << sizeof(Info2) << std::endl;
    std::cout << alignof(Info2) << std::endl;
    std::cout << sizeof(Info3) << std::endl;
    std::cout << alignof(Info3) << std::endl;

    exit(EXIT_SUCCESS);
}