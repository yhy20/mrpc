#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <mutex>

// 管理输入输出流的状态
 std::istream& func(std::istream& in, std::ostream& out, std::ostream& err)
 {
    // (1) 流类对象无拷贝或赋值，
    std::string str;
    while(in >> str)
    {
        std::cout << str << std::endl;
        // out << str << std::endl;
    }
    if(in.eof()) 
    {
        err << "End of stream!" << std::endl;
    }
    if(in.bad())
    {
        err << "Bad of stream!" << std::endl;
    }
    if (in.fail())
    {
        err << "Fail of stream!" << std::endl;
    }

    in.clear();
    out.clear();
    err.clear();
    return in;
 }

void test1()
{
    // istream 类的默认构造函数是保护类型，而带参数的构造函数则是公有的
    // explicit basic_istream(__streambuf_type* __sb) 构造函数需要传入一个 __streambuf_type*
    // typedef basic_streambuf<_CharT, _Traits> 		__streambuf_type;
    // 所以要定义一个istream 对象，必须要在参数中传入 streambuf 类型的指针
    // 由于 streambuf 只有一个默认构造函数且是保护类型，所以 streambuf 是不能直接定义一个对象的
    // 需要传入一个它的继承者 stringbuf 或者 filebuf


    // /dev/stderr -> /proc/self/fd/2
    // /dev/stdin  -> /proc/self/fd/0
    // /dev/stdout -> /proc/self/fd/1
    std::filebuf inbuf;
    std::filebuf outbuf;
    std::filebuf errbuf;
    // 问题？多个文件同时打开 /dev/stdin 会发生什么
    // 同时读写文件效率太低，一般是开一个内存缓冲取，然后多个线程同时读写
    // 分布式系统中，仍然是在内存中开一个缓冲区然后，同时读写，不会用文件
    // 文件锁理解
    if(inbuf.open("/dev/stdin", std::ios::in) == nullptr)
    {
        std::cerr << "open file error!" << std::endl;
        exit(-1);
    }
    
    if(outbuf.open("/proc/self/fd/1", std::ios::out) == nullptr)
    {
        std::cerr << "open file error!" << std::endl;
        exit(-1);
    }

    if(errbuf.open("/dev/sdterr", std::ios::out) == nullptr)
    {
        std::cerr << "open file error!" << std::endl;
        exit(-1);
    }

    std::istream in(&inbuf);
    std::ostream out(&outbuf);
    std::ostream err(&errbuf);
    func(in, out, err);
}


void test2()
{
    //
    std::filebuf inbuf;
    std::filebuf outbuf;
    if(inbuf.open("/dev/stdin", std::ios::in) == nullptr)
    {
        std::cerr << "open file error!" << std::endl;
        exit(-1);
    }
    
    if(outbuf.open("test2.out.txt", std::ios::out | std::ios::app) == nullptr)
    {
        std::cerr << "open file error!" << std::endl;
        exit(-1);
    }
   // 问题？？？？？？ 如何从键盘打出空格，回车，换行符号
    std::istream in(&inbuf);
    std::ostream out(&outbuf);
    // 开2个线程不加锁，同时写文件试试 
    std::string str;
    while(in >> str)
    {
        if(out.good())
        {
            out << str << std::endl;
        }
        else
        {
            std::cerr << "out error!" << std::endl;
        }   
    }
}


void test3()
{
    // 输出 hi 和一个换行，然后刷新缓冲区
    std::cout << "hi!" << std::endl;
    // 输出 hi 然后刷新缓冲区，不附加任何字符
    std::cout << "hi!" << std::flush;
    // 输出 hi 和一个空字符，然后刷新缓冲区
    std::cout << "hi!" << std::ends;

    // unitbuf 操作符
    // 如果想在每次输出操作后都刷新缓冲区，可以使用 unitbuf 操作符
    // 它告诉流在接下来的每次写操作后都进行一次 flush 操作。而 nounitbuf 
    // 操作符则重置流，使其恢复正常的缓冲取刷新机制
    std::cout << std::unitbuf;
    std::cout << std::nounitbuf;

    // ********* warning *********
    // 如果程序异常终止，输出缓冲区是不会被刷新的，当一个程序崩溃后
    // 它所输出的数据很可能停留在输出缓冲区中等待打印

    //
}


void test4()
{
    std::filebuf inBuf;
    std::filebuf outBuf;
    if(!inBuf.open("/dev/stdin", std::ios::in))
    {
        std::cerr << "open file error!" << std::endl;
        exit(-1);
    }
    
    if(!outBuf.open("/proc/self/fd/1", std::ios::out))
    {
        std::cerr << "open file error!" << std::endl;
        exit(-1);
    }

    std::istream in(&inBuf);
    std::ostream out(&outBuf);

    // tie() 函数返回指向输出流的指针
    // 如果本对象关联到一个输出流，则返回指向输出流的指针
    // 如果本对象未关联到一个输出流，则返回空指针
    std::ostream* outPtr = in.tie();
    if(!outPtr) out << "输入流未关联到输出流" << std::endl;

    // tie(&std::ostream p) 函数将本对象关联到传入的输出流，
    // 并返回之前关联的输出流对象的指针
    in.tie(&out);

    outPtr = in.tie();
    if(outPtr) out << "输入流关联到了输出流" << std::endl;

    std::string str;
    out << "please input a number:";
    // sleep 3 秒，这 3 秒中时间内，输出流并没有被刷新
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    // 在执行输入前，会先刷新 out 的缓冲区
    in >> str;
    out << str << std::endl;
    

    std::filebuf buf;
    if(!buf.open("/proc/self/fd/1", std::ios::out))
    {
        std::cerr << "open file error!" << std::endl;
        exit(-1);
    }
    // 输出流关联到输出流
    std::ostream os(&buf);
    os.tie(&out);
    out << "os.tie(&out)";
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    os << "test!"<< std::endl;
}

namespace 
{
    std::filebuf outbuf;
    std::ostream out(&outbuf);
    std::mutex mtx;
}

void task1(std::ostream& out)
{
    for(int i = 0; i < 100; ++i)
    {
        std::lock_guard<std::mutex> lock(mtx);
        out << "thread1 run!" << std::endl;
    }
}
void task2(std::ostream& out)
{
    for(int i = 0; i < 100; ++i)
    {
        std::lock_guard<std::mutex> lock(mtx);
        out << "thread2 run!" << std::endl;
    }
}

void test5()
{
    // 注意理解，文件锁是进程级别的锁，
    // 现在所写的都是线程级别的锁
    outbuf.open("/proc/self/fd/1", std::ios::out);
    out << "test" << std::endl;
    std::thread t1(task1, std::ref(out));
    std::thread t2(task2, std::ref(out));
    t1.join();
    t2.join();
}


namespace 
{
    std::filebuf fileBuf;
    std::ofstream outFile;
}

void test6()
{
    // 此处通过父类引用调用 operator 函数实际就是父类的函数，根本没使用动态类型绑定
    outFile.open("test6.out.txt", std::ios::out);
    std::thread t1(task1, std::ref(outFile));
    std::thread t2(task2, std::ref(outFile));
    t1.join();
    t2.join();
}

void test7()
{
    // 文件模式 
    // in
    // out
    // app
    // ate
    // trunc
    // binary

    // 保留被 ofstream 打开的文件中已有的数据的唯一方式是显示指定 app 或 in 模式
    // ofstream 为什么可以用 in 模式打开




}

// stringstream
void test8()
{
    // sstream 头文件定义了三个类型来支持内存 I/O
    // 这些类型可以向 string 写入数据，从 string 读出数据，就向 string 是一个内存上的文件一样
    // 这三种类型分别是 istringstream, ostringstream, stringstream
    // sstream 中定义的类型都继承自 iostream, 处理继承的来的操作，sstream 中还增加了一些成员
    // 来管理与流相关联的 string, 具体如下
    // sstream strm;  
    // sstream strm(s);
    // strm.str();
    // strm.str(s);

    std::istringstream iss("12 abcd");
    int a;
    std::string str;
    iss >> a;
    iss >> str;
    std::cout << a << std::endl;
    std::cout << str << std::endl;

    std::ostringstream oss1;
    oss1 << "oss1:" << "thread id:" << std::this_thread::get_id() << std::endl;
    std::ostringstream oss2("oss2:");
    oss2 << "thread id:" << std::this_thread::get_id() << std::endl;
    std::cout << oss1.str();
    std::cout << oss2.str();

}


int main()
{   
    // test1();

    // test2();

    // test3();

    // test4();

    test5();

    // test6();

    // test8();
    return 0;
}