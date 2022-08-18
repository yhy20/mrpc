#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>

ucontext_t g_mainCtx, g_taskCtx;

void Task()
{
    printf("Task run!\n");
    setcontext(&g_mainCtx);
}

int main()
{
    char stack[1024];
    getcontext(&g_taskCtx);
    g_taskCtx.uc_stack.ss_sp = stack;
    g_taskCtx.uc_stack.ss_size = sizeof(stack);
    g_taskCtx.uc_link = nullptr;
    makecontext(&g_taskCtx, Task, 0);
    getcontext(&g_mainCtx);
    setcontext(&g_taskCtx);
    // swapcontext(&g_mainCtx, &g_taskCtx);

    printf("Main finish!\n");
    return 0;
}