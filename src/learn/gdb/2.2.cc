#include <stdio.h>

int func(void)
{
    return 3;
}

int main(void)
{
    int a = 0;
    a = func();
    printf("%d\n", a);
    return 0;
}