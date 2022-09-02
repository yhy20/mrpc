#include <stdio.h>

int main(void)
{
    int a = 100;
    int b = 200;
    const int* p1 = &a;
    int* const p2 = &a;
    p1 = &b;
    *p2 = b;
    printf("*p1 = %d, *p2 = %d\n", *p1, *p2);
}