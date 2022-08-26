#include <stdio.h>

namespace Foo
{
    void foo()
    {
        printf("foo\n");
    }
}

namespace 
{
    void bar()
    {
        printf("bar\n");
    }
}

int main(void)
{
    /// b Foo::foo
    Foo::foo();
    /// (anonymous namespace)::bar
    bar();
    return 0;
}