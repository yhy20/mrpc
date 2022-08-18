// #include <atomic>

// #include "Coroutine.h"

// namespace mrpc
// {

// static std::atomic<uint64_t> g_coroutineId(0);
// static std::atomic<uint64_t> g_coroutineCount(0);

// static thread_local Coroutine* t_coroutine = nullptr;
// static thread_local std::shared_ptr<Coroutine::Ptr> t_threadCoroutine = nullptr;

// static uint32_t g_CoroutineStackSize = 1024 * 1024;

// class MallocStackAllocator
// {
// public:
//     static void* Alloc(size_t size)
//     {
//         return malloc(size);
//     }
//     static void Dealloc(void* ptr, size_t size)
//     {
//         free(ptr);
//     }

// };

// using StackAllocator = MallocStackAllocator;

// Coroutine::Coroutine()
// {
//     m_state = EXEC;
//     SetThis(this);
    
//     if(getcontext(&m_ctx))
//     {
//         assert(false);
//     }

//     ++g_coroutineCount;

// }

// Coroutine::Coroutine(CoroutineTask task, size_t stackSize = 0)
//     : m_id(++g_coroutineId),
//       m_task(std::move(task)),
//       m_stackSize(stackSize != 0 ? stackSize : g_CoroutineStackSize)
// {
//     m_stack = StackAllocator::Alloc(m_stackSize);
//     if(getcontext(&m_ctx))
//     {
//         assert(false);
//     }
//     m_ctx.uc_link = nullptr;
//     m_ctx.uc_stack.ss_sp = m_stack;
//     m_ctx.uc_stack.ss_size = m_stackSize;

//     makecontext(&m_ctx, &Coroutine::MainFunc, 0);

// }

// Coroutine::~Coroutine()
// {
//     --g_coroutineCount;
//     if(m_stack) {
//         assert(m_state == TERM || m_state == INIT);
//         StackAllocator::Dealloc(m_stack, m_stackSize);
//     }
//     else
//     {
//         assert(!m_task);
//         assert(m_state == EXEC);
//         if(t_coroutine == this)
//         {
//             SetThis(nullptr);
//         }

//     }
// }

// /**
//  * @brief 设置当前协程
//  */
// static void SetThis(Coroutine* c);
// /**
//  * @brief 返回当前协程
//  */
// static Coroutine GetThis();
// /**
//  * @brief 协程切换到后台并设置 Ready 状态
//  */
// static void YieldTOReady();
// /**
//  * @brief 协程切换到后台并设置 Hold 状态
//  */
// static void YieldTOHold();
// /**
//  * @brief 创建的协程数
//  */
// static uint64_t CoroutineNum();


// }  // namespace mrpc