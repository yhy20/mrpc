# 五、标准 I/O 库
### 5.1 概述

### 5.3 标准输入流、标准输出流、标准错误流
**&emsp;&emsp;对一个进程预定义了 3 个流，并且这三个流可以自动的被进程使用，它们是标准输入，标准输出和标准错误。这些流与 STDIN_FILENO，STDOUT_FILENO、STDERR_FILENO 等文件描述符引用的文件相同（/dev/stdout, /dev/stdin, /dev/stderr 等）**

### 5.4 缓冲
**(1) 全缓冲**
**(2) 行缓冲**
**(3) 无缓冲**

### 5.9 二进制 I/O
**&emsp;&emsp;二进制 I/O 也是字节 I/O**