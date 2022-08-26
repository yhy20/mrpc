#include <stdio.h>

/// disassemble func
/// i frame
/// i registers
void func(int a, int b)
{
    int c = a * b;
    printf("c is %d\n", c);
}

int main(void)
{
    func(1, 2);
    return 0;
}

