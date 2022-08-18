#include "ucontext.h"
#include <iostream>
using namespace std;
 
ucontext_t parent,child;
 
void Fun2(void* arg)
{
    for(int i = 0;i < 5; i++){
        cout<<"child "<<i<<endl;
        // swapcontext(&child,&parent);
    }
}
 
void Fun1(void* arg)
{
    char stack[1024];
    
    getcontext(&child);
 
    child.uc_stack.ss_sp = stack;
    child.uc_stack.ss_size = sizeof(stack);
    /// child.uc_link = &parent;
    child.uc_link = nullptr;
 
    makecontext(&child,(void(*)(void))Fun2,0);
 
    setcontext(&child);
    // for(int i = 0;i < 5; i++){
    //     cout<<"parent "<<i<<endl;
    //     swapcontext(&parent,&child);
    // }
}
 
int main()
{
    Fun1(0);
    return 0;
}