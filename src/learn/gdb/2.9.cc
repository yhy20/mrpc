#include <stdio.h>

int func1(int a)
{
    return 2 * a;
}

int func2(int a)
{
    int c = 0;
    c = 2 * func1(a);
    return c;
}

int func3(int a)
{
    int c = 0;
    c = 2 * func2(a);
    return c;
}

/// up n, down n
int main(void)
{
    printf("%d\n", func3(10));
    return 0;
}

