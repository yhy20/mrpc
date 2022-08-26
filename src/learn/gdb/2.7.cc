#include <stdio.h>
void a(void)
{
    printf("Tail call frame\n");
}

void b(void)
{
    a();
}

void c(void)
{
    b();
}

/// set debug entry-values 1
int main(void)
{
    c();
    return 0;
}

