#include <stdio.h>

int func(void)
{
    int i = 0;
    i += 2;
    i *= 10;

    return i;
}

int main(void)
{
    /// finish 命令, 函数会继续执行完毕，并且打印返回值。
    /// return 命令, 函数不会继续执行下面的语句，而是直接返回。
    int a = 0;
    a = func();
    printf("%d\n", a);
    
    return 0;
}