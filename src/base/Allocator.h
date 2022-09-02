#ifndef __MRPC_BASE_ALLOC_H__
#define __MRPC_BASE_ALLOC_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/// 通过 MRPC_THROW_BAD_ALLOC 宏控制异常处理
#ifndef MRPC_THROW_BAD_ALLOC
#   if defined(MRPC_USE_EXCEPTIONS) || !defined(MRPC_NO_BAD_ALLOC)
#    include <new>
#    define MRPC_THROW_BAD_ALLOC throw std::bad_alloc()
#   else
#    include <stdio.h>
#    include <stdlib.h>
#    define MRPC_THROW_BAD_ALLOC fprintf(stderr, "out of memory\n"); exit(1)
#   endif
#endif


/// 通过 MRPC_THREADS 宏控制二级空间置配器的线程安全性
#if defined(MRPC_THREADS)
/// Thread-safe.
#include <thread>
#   define MRPC_NODE_ALLOCATOR_LOCK_GUARD std::lock_guard(m_node_allocator_lock)
#   define MRPC_VOLATILE volatile
#   define MRPC_NODE_ALLOCATOR_THREADS true

#else 
/// Thread-unsafe.
#   define MRPC_NODE_ALLOCATOR_LOCK_GUARD  
#   define MRPC_VOLATILE
#   define MRPC_NODE_ALLOCATOR_THREADS false
#endif

namespace mrpc
{

/**
 * @brief 直接 malloc 空间的一级空间置配器
 * @details 由于 malloc 本身线程安全，所以 MallocAllocTemplate 也线程安全
 * @tparam Inst 模板索引
 */
template <int Inst>
class MallocAllocTemplate
{
public:
    /// 内存溢出处理函数类型
    typedef void (*OutOfMemoryHandler)();

    /**
     * @brief 分配指定大小的空间
     * @param[in] n 分配的字节数
     */
    static void* allocate(size_t n)
    {
        void* result = malloc(n);
        if(nullptr == result) result = outOfMemoryMalloc(n);
        return result;
    }

    /**
     * @brief 将空间归还到内存池
     * @param[in] p 指向内存空间的指针
     * @param[in] n 该参数用于形式上吻合二级空间置配器
     */
    static void deallocate(void* p, size_t /* n */)
    {
        free(p);
    }

    /**
     * @brief 重新分配空间
     * @param[in] p 指向原空间的指针 
     * @param old_size 原空间大小
     * @param new_size 新空间大小
     */
    static void* reallocate(void* p, size_t /* oldSize */, size_t newSize)
    {
        void* result = realloc(p, newSize);
        if(nullptr == result) result = outOfMemoryRealloc(p, newSize);
        return result;
    }

    /**
     * @brief 设置内存溢出时的处理回调
     * @param f 内存溢出处理函数地址
     */
    static OutOfMemoryHandler setMallocHandler(OutOfMemoryHandler f)
    {
        OutOfMemoryHandler old = m_outOfMemoryHandler;
        m_outOfMemoryHandler = f;
        return old;
    }

private:
    /**
     * @brief 内存溢出时的 malloc 处理
     * @param n malloc 分配的字节数
     */
    static void* outOfMemoryMalloc(size_t n);
    /**
     * @brief 内存溢出时的 realloc 处理
     * @param p 指向原空间的指针
     * @param n 需要新分配的空间字节数
     */
    static void* outOfMemoryRealloc(void* p, size_t n);

private:
    /// 内存溢出处理回调
    static OutOfMemoryHandler m_outOfMemoryHandler;
};

template <int Inst>
typename MallocAllocTemplate<Inst>::OutOfMemoryHandler 
MallocAllocTemplate<Inst>::m_outOfMemoryHandler = nullptr;

template <int Inst>
void* MallocAllocTemplate<Inst>::outOfMemoryMalloc(size_t n)
{
    void* result;
    for(;;)
    {
        if(nullptr == m_outOfMemoryHandler) 
            { MRPC_THROW_BAD_ALLOC; }
        (*m_outOfMemoryHandler)();
        result = malloc(n);
        if(result) return result;
    }
}

template <int Inst>
void* MallocAllocTemplate<Inst>::outOfMemoryRealloc(void* p, size_t n)
{
    void* result;
    for(;;)
    {
        if(nullptr == m_outOfMemoryHandler)
            { MRPC_THROW_BAD_ALLOC; }
        (*m_outOfMemoryHandler)();
        result = realloc(p, n);
        if(result) return result;
    }
}

typedef MallocAllocTemplate<0> MallocAlloc;

/**
 * 定义 MRPC_USE_MALLOC 宏，则表示直接使用 malloc 
 * 进行内存分配，这意味着关闭默认启用的二级空间置配器
 */
#ifdef MRPC_USE_MALLOC

typedef MallocAlloc alloc;
typedef MallocAlloc single_clien_alloc;

#else

/**
 * @brief 默认使用的二级空间置配器
 * @tparam Threads 是否线程安全
 * @tparam Inst 模板索引
 */
template <bool Threads, int Inst>
class DefaultAllocTemplate
{
public:
    /**
     * @brief 分配指定大小的空间
     * @param[in] n 分配的字节数，n 必须大于 0
     */
    static void* allocate(size_t n)
    {
        void* ret = nullptr;
        
        /// 大于 128 bytes，则不用内存池管理，直接 malloc 分配
        if(n > kMaxBytes)
        {
            ret = MallocAlloc::allocate(n);
        }
        else
        { 
            /// 获取合适大小的 chunk 块的空闲链表起始位置
            Obj* MRPC_VOLATILE* myFreeList =
                s_freeList + freeListIndex(n);

            /// 多线程下环境下加锁
            MRPC_NODE_ALLOCATOR_LOCK_GUARD;

            /// 获取空闲链表指向的空闲空间
            Obj* result = *myFreeList;
            /// 若空闲链表中无空闲的空间，则重新填充空闲链表（不一定需要分配新的内存）
            if(result == nullptr)
            {
                ret = refill(roundUp(n));
            }
            /// 若空闲链表中有空闲的空间，则空闲链表向后挪动
            else
            {
                *myFreeList = result->m_freeListLink;
                ret = result;
            }
        }
        return ret;
    }

    /**
     * @brief 将空间归还到内存池
     * @param[in] p 指向内存空间的指针，p 必须非空
     * @param[in] n 归还的字节数
     */
    static void deallocate(void* p, size_t n)
    {
        /// 大于 128 bytes，则不用内存池管理，直接 free 回收
        if(n > kMaxBytes)
        {
            MallocAlloc::deallocate(p, n);
        }
        /// 将空间接到空闲链表头部即可
        else
        { 
            Obj* MRPC_VOLATILE* myFreeList = s_freeList + freeListIndex(n);
            Obj* q = reinterpret_cast<Obj*>(p);

            MRPC_NODE_ALLOCATOR_LOCK_GUARD;

            q->m_freeListLink = *myFreeList;
            *myFreeList = q;
        }
    }

    /**
     * @brief 重新分配空间
     * @param[in] p 指向原空间的指针 
     * @param old_size 原空间大小
     * @param new_size 新空间大小
     */
    static void* reallocate(void* p, size_t old_size, size_t new_size);

private:
    /// 空闲链表结点的指针域
    union Obj
    {
        union Obj* m_freeListLink;
        char m_clientData[1];   // The client sees this.
    };

private:
    /**
     * @brief 将 bytes 上调至 8 的倍数
     * @details if bytes == 0, return 0
     * @param[in] bytes 需要分配的字节数
     */
    static size_t roundUp(size_t bytes)
    {
        /**
         * 例如 bytes = 5
         * bytes               => 0000 0000 0000 0101
         * bytes + kAlign - 1  => 0000 0000 0000 1100
         * ~(kAlign - 1)       => 1111 1111 1111 1000
         * return              => 0000 0000 0000 1000
         * 
         * 构成如下映射关于，并依此类推
         * 1 ~ 8   => 8
         * 9 ~ 16  => 16
         * 17 ~ 24 => 24
         */
        return ((bytes + kAlign - 1) & ~(kAlign - 1));
    }
    
    /**
     * @brief 返回分配 bytes 大小的内存需要的空闲链表头部位于 freeList 的位置
     * @details if bytes == 0, reutrn -1
     * @param bytes  需要分配的字节数
     */
    static size_t freeListIndex(size_t bytes)
    {
        return ((bytes + kAlign - 1) / kAlign - 1); 
    }

    /**
     * @brief 重新填充空闲链表
     * @param n allocate 申请的内存大小
     */
    static void* refill(size_t n);

    /**
     * @brief 以字节为单位分配 chunk 块内存
     * @param size 空闲链表上 8 字节对齐 Obj 的大小（8 => 16 => 24 => ... => 120 => 128)
     * @param nobjs Obj 的期望数目
     */
    static char* chunkAlloc(size_t size, int& nobjs);

private:
    static const size_t kAlign = 8;         // 对齐谓数
    static const size_t kMaxBytes = 128;    // 内存池管理的最大对象字节
    static const size_t kNfreeLists = 16;   // 空闲链表数目（kMaxBytes / kAlign）

    static char*  s_startFree;   // 备用空间起始地址
    static char*  s_endFree;     // 备用空间结束地址
    static size_t s_heapSize;    // 已分配堆内存总数

#ifdef MRPC_THREADS
    static std::mutex    m_node_allocator_lock;
#endif
    static Obj* MRPC_VOLATILE s_freeList[kNfreeLists];
};


typedef DefaultAllocTemplate<MRPC_NODE_ALLOCATOR_THREADS, 0> alloc;
typedef DefaultAllocTemplate<false, 0> single_client_alloc;

template <bool Threads, int Inst>
inline bool operator==(const DefaultAllocTemplate<Threads, Inst>&, 
                       const DefaultAllocTemplate<Threads, Inst>&)
{
    return true;
}

template <bool Threads, int Inst>
inline bool operator!=(const DefaultAllocTemplate<Threads, Inst>&, 
                       const DefaultAllocTemplate<Threads, Inst>&)
{
    return false;
}


template <bool Threads, int Inst>
void* DefaultAllocTemplate<Threads, Inst>::reallocate(void* p, 
                                                      size_t old_size,
                                                      size_t new_size)
{
    void* result;
    size_t copy_size;
    
    /**
     * 若 old_size 和 new_size 均大于 kMaxBytes，则这两块
     * 内存的管理与二级空间置配器无法，直接使用 realloc 分配
     */
    if(old_size > kMaxBytes && new_size > kMaxBytes)
    {
        return realloc(p, new_size);
    }
    /**
     * 若 roundUp(old_size) == roundUp(new_size)，则说明
     * 两块内存均小于 kMaxBytes 且均由二级空间置配器直接管理，
     * 由于二级空间置配器分配的内存都是 8 字节对齐的，所以实际
     * 上 old_size 使用的空间大小为 roundUp(old_size)，也就
     * 显然满足 new_size 的需求，直接返回原空间 p 即可
     */
    if(roundUp(old_size) == roundUp(new_size)) return p;
    
    /// 针对 new_size 分配新的空间
    result = allocate(new_size);

    /// 尽可能的拷贝数据 
    copy_size = new_size > old_size ? old_size : new_size;
    memcpy(result, p, copy_size);

    /// 释放 old_size 使用的空间
    deallocate(p, old_size);
    return result;
}


template <bool Threads, int Inst>
void* DefaultAllocTemplate<Threads, Inst>::refill(size_t n)
{
    /**
     * 分配一个 chunk 块，期望大小为 nobjs * n 字节
     * 需要注意的是 nobjs = 20 只是一个根据经验设置
     * 期望值，在 chunkAlloc 函数中可能会使用 malloc
     * 分配新的空间并返回期望的大小的空间，也可能会从
     * 备用空间中划分一块并调整 nobjs 的值，若返回的 
     * nobjs 的值大于 1， 则会 build freeList, 否则
     * 直接返回分配的空间，不会 build freeList
     */
    int nobjs = 20;
    char* chunk = chunkAlloc(n, nobjs);

    /// 若 nobjs == 1，直接 return，并且不  build freeList
    if(1 == nobjs) return chunk;

    Obj* MRPC_VOLATILE* myFreeList = s_freeList + freeListIndex(n);
    /// result 是最终的返回值，不会被 build 为 freeList 的一部分
    Obj* result = reinterpret_cast<Obj*>(chunk);

    /// 在 chunk 块上循环构建空闲链表 
    Obj *currentObj, *nextObj;
    *myFreeList = nextObj = reinterpret_cast<Obj*>(chunk + n);
    for(int i = 0; ; ++i)
    {
        currentObj = nextObj;
        nextObj = reinterpret_cast<Obj*>(reinterpret_cast<char*>(nextObj) + n);
        if(nobjs - 1 == i)
        {
            currentObj->m_freeListLink = nullptr;
            break;
        }
        else
        {
            currentObj->m_freeListLink = nextObj;
        }
    }

    return static_cast<void*>(result);
}


template <bool Threads, int Inst>
char* DefaultAllocTemplate<Threads, Inst>::chunkAlloc(size_t size, 
                                                      int& nobjs)
{
    char* result;
    /// 计算需要分配的总字节数
    size_t totalBytes = size * nobjs;
    /// 计算 chunk 块剩余的空闲字节数
    size_t bytesLeft = s_endFree - s_startFree;

    /// chunk 块剩余空间满足全部需求，则分配全部空间
    if(bytesLeft >= totalBytes)
    {
        result = s_startFree;
        s_startFree += totalBytes;
        return result;
    }
    /// chunk 块剩余空间只满足部分需求，则分配部分空间
    else if(bytesLeft >= size) 
    {
        nobjs = static_cast<int>(bytesLeft / size);
        totalBytes = size * nobjs;
        result = s_startFree;
        s_startFree += totalBytes;
        return result;
    }
    /**
     * chunk 块剩余空间不足 size, 则需要 malloc 分配一块新空间
     * 该空间的大小至少为 totalBytes 的两倍，其中前 totalBytes
     * 的空间用于构建空闲链表，剩余的空间由状态 s_startFree 和 
     * 状态 s_endFree 记录备用
     * PS: 首次执行 chunkAlloc 函数一定会进入 else 分支
     */
    else
    {
        /// roundUp(s_heapSize >> 4) 表示根据已分配 heap 内存动态多分配的空间 
        size_t bytesToGet = 2 * totalBytes + roundUp(s_heapSize >> 4);

        /**
         * 尽量尝试完全利用剩余的备用空间，需要注意，备用空间的开辟是 8 字节对齐
         * 的，所以一定能在 freeList 找到一个位置（至少最 8 字节的 freeList)，
         * 所以下面的操作是一定能够成功的。唯一的问题是可能无法利用全部的剩余备用
         * 空间，例如剩余 40 个字节，此时会浪费 8 个字节的空间。
         */
        if(bytesLeft > 0)
        {
            Obj* MRPC_VOLATILE* myFreeList = 
                s_freeList + freeListIndex(bytesLeft);
            
            Obj* currentObj = reinterpret_cast<Obj*>(s_startFree);
            currentObj->m_freeListLink = *myFreeList;
            *myFreeList = currentObj; 
        }
        
        /// 尝试分配 bytesToGet 字节的空间
        s_startFree = reinterpret_cast<char*>(malloc(bytesToGet));

        /// malloc 空间分配失败
        if(nullptr == s_startFree)
        {
            Obj* MRPC_VOLATILE* myFreeList;
            Obj* p;

            for(size_t i = size; i <= kMaxBytes; i += kAlign)
            {
                myFreeList = s_freeList + freeListIndex(i);
                p = *myFreeList;
                if(p != nullptr)
                {
                    /// 从 i 大小的 freeList 中取用一个节点作为备用空间
                    *myFreeList = p -> m_freeListLink;
                    s_startFree = reinterpret_cast<char*>(p);
                    s_endFree = s_startFree + i;
                    return chunkAlloc(size, nobjs);
                }
            }
            s_endFree = 0;
            /// MallocAlloc::allocate() 函数大概率会分配失败，失败后走异常流程
            s_startFree = reinterpret_cast<char*>(MallocAlloc::allocate(bytesToGet));
        }

        /// 更新 s_heapSize，s_endFree 的值
        s_heapSize += bytesToGet;
        s_endFree = s_startFree + bytesToGet;
        return chunkAlloc(size, nobjs);
    }
}

#ifdef MRPC_THREADS
template <bool Threads, int Inst>
std::mutex DefaultAllocTemplate<Threads, Inst>::m_node_allocator_lock;
#endif

template <bool Threads, int Inst>
char* DefaultAllocTemplate<Threads, Inst>::s_startFree = nullptr;

template <bool Threads, int Inst>
char* DefaultAllocTemplate<Threads, Inst>::s_endFree = nullptr;

template <bool Threads, int Inst>
size_t DefaultAllocTemplate<Threads, Inst>::s_heapSize = 0;

template <bool Threads, int Inst>
typename DefaultAllocTemplate<Threads, Inst>::Obj* MRPC_VOLATILE
DefaultAllocTemplate<Threads, Inst>::s_freeList[
    DefaultAllocTemplate<Threads, Inst>::kNfreeLists
] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

#endif  // MRPC_USE_MALLOC

#define MRPC_STD_ALLOCATORS

#ifdef MRPC_STD_ALLOCATORS

/**
 * @brief 符合 C++ 标准的 Allocator 实现
 */
template <typename Tp>
class Allocator
{
    /// 定义底层实际使用的空间置配器
    typedef alloc _Alloc;
public:
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef Tp*          pointer;
    typedef const Tp*    const_pointer;
    typedef Tp&          reference;
    typedef const Tp&    const_reference;
    typedef Tp           value_type;

    template <typename Tp1> struct rebind
    {
        typedef Allocator<Tp1> other;
    };

    Allocator() { }
    Allocator(const Allocator<Tp>&) { }
    template <typename Tp1> 
    Allocator(const Allocator<Tp1>&) { }
    ~Allocator() { }

    pointer address(reference x) const { return &x; }
    const_pointer address(const_pointer x) const { return &x; }

    Tp* allocate(size_type n, const void* = nullptr)
    {
        return (n != 0) ? static_cast<Tp*>(_Alloc::allocate(n * sizeof(Tp)))
                        : nullptr;
    }

    void deallocate(pointer p, size_type n)
        { _Alloc::deallocate(p, n * sizeof(Tp)); }

    size_type max_size() const
        { return static_cast<size_t>(-1) / sizeof(Tp); }
    
    void construct(pointer p, const Tp& val) { new(p) Tp(val); }
    void destroy(pointer p) { p->~Tp(); }
};

template <>
class Allocator<void>
{
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef void*       pointer;
    typedef const void* const_pointer;
    typedef void        value_type;

    template <class U> struct rebind
    {
        typedef Allocator<U> other;
    };
};

template <class T1, class T2>
inline bool operator==(const Allocator<T1>&, const Allocator<T2>&) 
{
    return true;
}

template <class T1, class T2>
inline bool operator!=(const Allocator<T1>&, const Allocator<T2>&)
{
    return false;
}

#endif  // MRPC_BASE_ALLOCATORS

}  // namespace mrpc;

#endif  // __MRPC_BASE_ALLOC_H__