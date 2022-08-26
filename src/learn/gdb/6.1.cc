#include <stdio.h>
#include <wchar.h>

int main(void)
{
#ifdef _T
    printf("in\n");
#endif 
    // https://blog.csdn.net/amusi1994/article/details/53898960
    char str1[] = TEXT("abdc");
    wchar_t str2[] = L"abdc";

    return 0;
}

