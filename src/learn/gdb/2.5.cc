#include <stdio.h>

int global = 1;

int func(void)
{
    return (++global);
}

int main(void)
{   
    /// call func()
    /// print func()
    printf("%d\n", global);
    return 0;
}