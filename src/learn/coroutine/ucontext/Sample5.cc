#include <stdio.h>
#include <ucontext.h>

ucontext_t main_ctx;

void context1_func(void)
{
    printf("start context1 hello world\n");
} 
void context2_func(void)
{
    printf("start context2 hello world\n");
} 

void init_coroutine(ucontext_t *ctx, void (*func) (void), char *stack, int stack_size);

int main(void)
{
    ucontext_t context1, context2;
    char stack1[8192];
    init_coroutine(&context1, context1_func, &stack1[0], sizeof(stack1));   
    char stack2[8192];
    init_coroutine(&context2, context2_func, &stack2[0], sizeof(stack2));
    // 将当前 context 保存到 ctx[0]，切换到 ctx[1]
    swapcontext(&main_ctx, &context1);
    printf("I'm in main\n");
    swapcontext(&main_ctx, &context2);
    printf("I'm in main\n");
    return 0;
}  

void init_coroutine(ucontext_t *ctx, void (*func) (void), char *stack, int stack_size)
{
    ctx->uc_stack.ss_sp = stack;
    ctx->uc_stack.ss_size = stack_size;
    // 注意此行代码，设置 uc_link
    ctx->uc_link = &main_ctx; 
    makecontext(ctx, func, 0);
}