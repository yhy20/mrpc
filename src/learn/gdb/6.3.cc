
/// gdb 中要打印大数组内容，则缺省最多会显示 200 个元素
int main()
{
    int array[201];
    for(int i = 0; i < 201; ++i)
    {
        array[i] = i;
    }

    return 0;
}