// #ifndef __MRPC_BASE_COROUTINE_H__
// #define __MRPC_BASE_COROUTINE_H__

// #include <ucontext.h>

// #include <memory>
// #include <functional>

// #include "Thread.h"

// namespace mrpc
// {

// class Coroutine : public std::enable_shared_from_this<Coroutine>
// {
// private:
//     Coroutine();

// public:
//     typedef std::function<void()> CoroutineTask;
//     typedef std::shared_ptr<Coroutine> Ptr;
//     enum State
//     {
//         INIT,   // 初始状态
//         HOLD,   // 暂停状态
//         EXEC,   // 执行状态
//         TERM,   // 结束状态
//         READY   // 可执行状态
//     };

//     Coroutine(CoroutineTask task, size_t stackSize = 0);
//     ~Coroutine();

//     /**
//      * @brief 重置协程函数，并重置协程状态
//      */
//     void reset(CoroutineTask task);
//     /**
//      * @brief 切换到当前协程执行
//      */
//     void swapIn();
//     /**
//      * @brief 将当前协程切换到后台执行
//      */
//     void swapOut();

// public:
//     /**
//      * @brief 设置当前协程
//      */
//     static void SetThis(Coroutine* co);
//     /**
//      * @brief 返回当前协程
//      */
//     static Coroutine GetThis();
//     /**
//      * @brief 协程切换到后台并设置 Ready 状态
//      */
//     static void YieldTOReady();
//     /**
//      * @brief 协程切换到后台并设置 Hold 状态
//      */
//     static void YieldTOHold();
//     /**
//      * @brief 创建的协程数
//      */
//     static uint64_t CoroutineNum();

//     static void MainFunc();



// private:
//     uint64_t m_id;
//     uint32_t m_stackSize = 0;
//     State m_state = INIT;

//     ucontext_t m_ctx;
//     void* m_stack = nullptr;
//     CoroutineTask m_task;
// };


// }  // namespace mrpc

// #endif  // __MRPC_BASE_COROUTINE_H__