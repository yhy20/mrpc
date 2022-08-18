#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>

ucontext_t ctx[2];

void Test()
{
    printf("Test start!\n");
    swapcontext(&ctx[1], &ctx[0]);

    printf("Test run!\n");
    swapcontext(&ctx[1], &ctx[0]);

    printf("Test stop!\n");
    // swapcontext(&ctx[1], &ctx[0]);
} 

int main(void)
{   
    /// swapcontext() 实现将当前的 context 保存到 oucp 中，并切换到 ucp 指向 context 继续执行
    /// swapcontext (ucontext_t *__restrict __oucp, const ucontext_t *__restrict __ucp)

    char stack[2048];

    /// 不使用 getcontext 函数会发生 segmentfault
    getcontext(&ctx[1]);
    ctx[1].uc_stack.ss_sp = stack;
    ctx[1].uc_stack.ss_size = sizeof(stack);
    // ctx[1].uc_link = &ctx[0]; 
    makecontext(&ctx[1], Test, 0);

    printf("Main start!\n");
    swapcontext(&ctx[0], &ctx[1]);

    printf("Main run!\n");
    swapcontext(&ctx[0], &ctx[1]);

    printf("Main stop!\n");
    ctx[1].uc_link = &ctx[0];
    swapcontext(&ctx[0], &ctx[1]);

    printf("Finish!\n");
    return 0;
}