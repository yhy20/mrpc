#include <stdio.h>

int main(void)
{
    /// start
    int i = 0;
    int sum = 0;
    for(i = 1; i < 200; ++i)
    {
        /// b 11 if i==101 , p sum
        sum += i;
    }

    /// ignore bnum count
    printf("%d\n", sum);
    return 0;
}