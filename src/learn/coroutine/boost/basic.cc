#include <iostream>
#include <boost/coroutine2/all.hpp>

void CoroutineTask1(boost::coroutines2::coroutine<void>::pull_type& coroutineBack)
{
    std::cout << "a ";
    coroutineBack(); // 锚点，返回
    std::cout << "b ";
    coroutineBack(); // 锚点 返回
    std::cout << "c " << std::endl;

    std::cout << "Coroutine finish!" << std::endl;
}

void Test1()
{
    boost::coroutines2::coroutine<void>::push_type coroutineObject(CoroutineTask1); 	// 创建协程
    std::cout << "1 ";
    coroutineObject(); // 运行协程
    std::cout << "2 ";
    coroutineObject(); // 返回锚点，继续运行协程
    std::cout << "3 ";
    coroutineObject(); // 返回锚点，继续运行协程

    /// 协程运行完毕后会返回 Test1 继续执行
    std::cout << "Test1 finish!" << std::endl; 
}

int main()
{
    Test1();

    // Test2();
    return 0;
}