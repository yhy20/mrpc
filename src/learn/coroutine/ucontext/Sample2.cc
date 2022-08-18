#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>

void Task()
{
    printf("Task run!\n");
}


void Test1()
{
    /// void makecontext (ucontext_t *__ucp, void (*__func) (void), int __argc, ...) 生成指定的上下文 
    ucontext_t taskCtx;
    char stack[1024];

    getcontext(&taskCtx);
    /// 初始化栈顶指针
    taskCtx.uc_stack.ss_sp = stack;
    /// 栈大小
    taskCtx.uc_stack.ss_size = sizeof(stack);
    /// 不指定下一个上下文
    taskCtx.uc_link = nullptr;

    /// 指定待执行的函数入口，并且该函数不需要参数
    makecontext(&taskCtx, Task, 0);
    setcontext(&taskCtx);

    /// 后续代码不会再执行了
    printf("Main finish!\n");    
}

void Test2()
{
    ucontext_t mainCtx, taskCtx;
    char stack[1024];

    getcontext(&taskCtx);
    /// 初始化栈顶指针
    taskCtx.uc_stack.ss_sp = stack;
    /// 栈大小
    taskCtx.uc_stack.ss_size = sizeof(stack);
    /// 不指定下一个上下文
    taskCtx.uc_link = &mainCtx;

    /// 指定待执行的函数入口，并且该函数不需要参数
    makecontext(&taskCtx, Task, 0);
    swapcontext(&mainCtx, &taskCtx);

    printf("Main finish!\n");    
}

/// 要通过协程执行某个函数，必须要提供栈空间来存放函数调用的栈帧信息
/// 思考栈空间不够怎么办
int main()
{
    // Test1();
    Test2();

    return 0;
}
