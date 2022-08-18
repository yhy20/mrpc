#include <vector>
#include <memory>
#include <functional>

#include "Types.h"
#include "StringPiece.h"
#include "noncopyable.h"

namespace mrpc
{
namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    /**
     * @brief TcpConnection 分发到的 I/O EventLoop 线程池
     * @param[in] baseLoop Acceptor EventLoop
     * @param[in] name 线程池名称
     */
    EventLoopThreadPool(EventLoop* baseLoop, StringArg name);

    /**
     * @brief 线程池中的线程
     *
     * 
     */
    ~EventLoopThreadPool();

    /**
     * @brief 非线程安全，设置线程池的线程数目，必须在 start 调用前设置
     * @param[in] numThreads 期望的线程数目
     */
    void setThreadNum(int numThreads) { m_numThreads = numThreads; }

    /**
     * @brief 非线程安全且仅能被调用一次，创建并初始化所有线程
     * @param[in] cb 传入的线程初始化回调
     */
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    /**
     * @brief 以轮询的方式获取每个线程的 EventLoop，用于分发 TcpConnection
     */
    EventLoop* getNextLoop();

    /**
     * @brief 获取特定 hashCode 对应线程的 EventLoop（简单的 Index 哈希）
     * @param[in] hashCode 哈希码  
     */
    EventLoop* getLoopForHash(size_t hashCode);

    /**
     * @brief 返回所有的 EventLoop 对象
     */
    std::vector<EventLoop*> getAllLoops();

    /**
     * @brief 线程池是否启动 
     */
    bool started() const { return m_started; }

    /**
     * @brief 线程安全函数，返回线程池名称
     */
    const std::string& name() const { return m_name; }

private:
    EventLoop*  m_baseLoop; 
    std::string m_name; 
    bool        m_started; 
    int         m_numThreads; 
    int         m_next; 
    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop*> m_loops;
};

}  // namespace net
}  // namespace mrpc