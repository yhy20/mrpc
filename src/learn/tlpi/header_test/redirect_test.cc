#include <string>
#include <iostream>

/**
 * 在输出重定向中 > 代表的是覆盖原文件而 >> 代表的是追加到文件，输出重定向的完整写法其实是
 * fd > file 或者 fd >> file，其中 fd 表示文件描述符，如果不写则默认为 1，也就是标准输出。
 * 当文件描述符为 1 时，一般都省略不写，当然也可以将 command > file 写作 command 1> file。
 * fd 和 > 之间不能有空格，否则 Shell 会解析失败， > 和 file 之间的空格可有可无，例如下面
 * echo "c.biancheng.net" 1 > tmp.txt 和 echo "c.biancheng.net" 1> tmp.txt
 * (1) 重定向标准输出和标准错误
 * ./redirect_test > ./tmp.txt
 * ./redirect_test >> ./tmp.txt
 * ./redirect_test 2> ./tmp.txt
 * ./redirect_test 2>> ./tmp.txt
 * ./redirect_test > /dev/null 
 * ./redirect_test 2> /dev/null
 * 
 * ./redirect_test > ./tmp.txt 2>&1
 * ./redirect_test >> ./tmp.txt 2>&1
 * ./redirect_test > out.log 2> err.log
 * ./redirect_test >> out.log 2>> err.log
 * ./redirect_test > /dev/null 2>&1
 * (wrong) ./redirect_test >./tmp.txt 2>./tmp.txt
 * (wrong) ./redirect_test >>./tmp.txt 2>>./tmp.txt
 * 
 * 和输出重定向类似，输入重定向的完整写法是fd < file，其中 fd 表示文件描述符，如果不写，默认为 0，
 * 也就是标准输入文件
 * (1) 重定向标准输入 
 * ./redirect_test < data.txt // 将 data.txt 文件中的内容作为 ./redirect_test 的输入
 * ./redirect_test < data.txt > ./tmp.txt 2>&1
 * 
 * command << END
 * 从标准输入（键盘）中读取数据，直到遇见分界符 END 才停止（分界符可以是任意的字符串，用户自己定义）。
 * 例如：wc -l << END 或者 ./redirect_test << END
 * 
 * ps -eo pid,ppid,sid,tty,pgrp,comm | grep -E 'bash|PID|sleep'
 */
int main()
{
    std::string inputStr;
    std::cin >> inputStr;
    for(int i = 0; i < 10; ++i)
    {
        std::cout << "stdout: " << inputStr << std::endl;
        std::cerr << "stderr: " << inputStr << std::endl;
    }
    exit(EXIT_SUCCESS);
}