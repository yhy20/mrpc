#include <stdio.h>

void print_a(void)
{
    puts("a");
}

void print_b(void)
{
    puts("b");
}

/// save breakpoints bt
/// source bt
/// tbreak 临时断点（tb)
int main(void)
{
    print_a();
    print_b();

    return 0;
}