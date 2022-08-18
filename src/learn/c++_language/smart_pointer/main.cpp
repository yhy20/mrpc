// #include "stdafx.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <memory>

namespace _nmsp1 {
	class A {
	public:
		A() {
			std::cout << "A::A() 执行！" << std::endl;
		}
		~A() {
			std::cout << "A::~A() 执行！" << std::endl;
		}
	};

	std::shared_ptr<int> make_int(int value) {
		return std::shared_ptr<int>(new int(value));
	}

	void func_value(std::shared_ptr<int> tv) {

	}

	void func_reference(std::shared_ptr<int>& tv) {

	}

	std::shared_ptr<int> func_return_value(std::shared_ptr<int>& tv) {
		return tv;
	}
}

namespace _nmsp2 {
	// 整型堆对象删除器
	void deleter(int* p) {
		std::cout << "整型 deleter 执行！" << std::endl;
		// 如果选择取代系统缺省的删除器，则有义务自己删除对象
		delete p;
	}
	
	// 封装 shared_ptr 数组
	template<typename T>
	std::shared_ptr<T> make_shared_array(size_t size) {
		return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
	}

	void func(std::shared_ptr<int> p) {

	}
}

namespace _nmsp3 {
	class A {
	public:
		std::shared_ptr<A> get_self() {
			return std::shared_ptr<A>(this);
		}
		// this 指针相当于指向对象的裸指针
		A* get_this() {
			return this;
		}
	};

	class SA : public std::enable_shared_from_this<SA>
	{
	public:
		std::shared_ptr<SA> get_self() {
			return shared_from_this();
		}
	};


	class CB;
	class CA {
	public:
		std::weak_ptr<CB> m_b;
		~CA(){
			std::cout << "CA::~CA() 执行！" << std::endl;
		}
	};

	class CB {
	public:
		std::shared_ptr<CA> m_a;
		~CB() {
			std::cout << "CB::~CB() 执行！" << std::endl;
		}
	};
	
	auto func() {
		return std::unique_ptr<std::string>(new std::string("Hello!"));
	}

	auto return_unique() {
		// 返回一个局部 unique_ptr 对象（将亡值）
		// 一般情况下系统会生成一个临时的对象来返回（如果有移动构造函数的话，会调用移动构造函数）
		return std::unique_ptr<std::string>(new std::string("Hello!"));
	}
}

namespace _nmsp4 {
	void deleter_function(std::string*& p) {
		std::cout << "delter_function() 执行！" << std::endl;
		delete p;
		p = nullptr;
	}
	
	class deleter_class {
	public:
		void operator()(std::string*& p) {
			std::cout << "delter_class() 执行！" << std::endl;
			delete p;
			p = nullptr;
		}
	};
}

void test1() {
	// 一、直接使用 new / delete 管理内存
	int* p1 = new int;
	// 值初始化：使用 () 或 {} 进行初始化
	// 对于内置类型，使用值初始化 new 出来的内存初值为 0
	int* p2 = new int();
	int* p3 = new int{};
	int* p4 = new int{ 100 };

	// 对于类对象，new 实际调用的是构造函数
	std::string* p5 = new std::string();
	std::string* p6 = new std::string("abc");
	std::string* p7 = new std::string(5, 'a');
	std::vector<int>* p8 = new std::vector<int>{ 1,2,3,4,5 };

	// 对于类对象，值初始化调用的默认构造函数
	_nmsp1::A* p9 = new _nmsp1::A;
	_nmsp1::A* p10 = new _nmsp1::A();

	// 指向常量的指针
	const int* cp1 = new const int(100);
	// 指向常量的常量指针
	const int* const cp2 = new const int(200);
	cp1 = new const int(300);
	// cp2 是指向常量的常指针不能被赋值
	// cp2 = new const int(400);

	// 注意，delete 后 p2 是野指针，指向未知内存，而不是 nullptr
	delete p2;
	// 最好在 delete 后将 p2 赋值为空指针
	p2 = nullptr;

	int* tp1 = new int(100);
	int* tp2 = tp1;
	// delete tp1 后，tp1 指向未知内存， tp2还是指向原内存，但内置中的值变成了未知值
	delete tp1;

	// 总结
	// (1) new 出的内存没有 delete 会导致内存泄露，内存泄露到一定程度会导致程序崩溃
	// (2) delete 后的内存不能再使用，否则会发生异常
	// (3) delete 某个指针后，将指针置为 nullptr
	// (4) 不能同时 delete 一块内存 2 次 
}

void test2() {
	// 二、new / delete 探秘与智能指针概述、shared_ptr 基础
	// (2.1) new / delete 探秘
	// sizeof 是（关键字 / 运算符），不是一个函数
	// malloc, free 主要用于 C 语言，因为 malloc 不能调用类的构造函数生成对象
	// new 和 delete 主要用于 C++
	// new 不但分配内存，还会做一下额外初始化工作，delete 不但释放内存，还会做一些额外的清理工作
	
	// new 调用构造函数，对堆上分配的对象进行初始化
	_nmsp1::A* p = new _nmsp1::A();
	// new 调用析构函数，将堆上的对象析构后释放内存
	delete p;

	// (2.2) operator new() 和 operator delete()
	// new 先分配内存（通过 operator new() 来分配内存），然后调用类的构造函数来初始化内存
	// void* operator new(size_t _Size) 用于分配原始内存
	void* ptr = operator new(100);
	// delete 先调用析构函数再释放内存（通过 operator delete() 来释放内存）
	// void operator delete(void* _Block) 用于释放分配的内存
	operator delete(ptr);

	// (2.3) 基本 new 如何记录分配的内存大小供 delete 使用
	// 不同编译器 new 内部实现方式不同
	int* p1 = new int; 
	// delete 如何知道需要回收的内存的大小（new 内部有记录机制）
	delete p1;

	// (2.4) 申请和释放一个数组
	int* p2 = new int(100);
	// int* p3 = new int[2]; 区别？？？？
	// 内置类型不需要记录 new 出的 int 个数
	int* p3 = new int[2]();
	// 调用 1 次构造函数
	_nmsp1::A* p4 = new _nmsp1::A();
	// 类 A 多出了 4 字节 
	// _nmsp1::A* p5 = new _nmsp1::A[2]; 区别？？？？
	// 调用 2 次构造函数
	_nmsp1::A* p5 = new _nmsp1::A[2]();

	delete p2;
	delete[] p3;
	// 调用 1 次析构函数
	delete p4;
	// 调用 2 次析构函数，多出的 4 字节用于记录 new 的对象个数
	delete[] p5;

	// (2.5) 为什么 new / delete 、new[] / delete[] 要配对使用
	// 内置类型如 int 不需要调用构造函数，所以 new[] 时候系统没有分配多出的 4 字节
	// 所以对于 int 类型，对于 new[]，调用 delete 或 delete[] 都一样

	// 对于一个对象，使用 new[] 来分配内存，却使用单独的 delete（而不是 delete[]）来释放内存，需要满足下面的某个条件
	// 1) 对象的类型是内置类型
	// 2) 对象的类型是类类型但没有自定义的析构函数（因为 new[] 时没有额外分配的 4 字节用于记录分配的类对象个数）

	// 为什么类提供自己的析构函数，不用 delete[] 释放内存会导致异常
	// 36 节 1 小时刻复看内存释放
	// 思考？？？？？？？？？？？ 深入研究 operator new 和 operator delete 的实现
}

void test3_1() {
	// new delete 的问题
	// a) delete 前抛出异常，或者触发分支，导致忘记 delete
	// b) 多个指针指向同一个内存导致的协同问题

	// 智能指针对 "裸指针" 进行包装，能够自动释放 new 出来的内存
	// (3.1.1) 标准库中 4 种智能指针的类型
	// auto_ptr, unique_ptr, shared_ptr, weak_ptr
	// 帮助管理动态分配内存对象的生命周期，有效防止内存泄露
	
	// shared_ptr：共享式指针（共享对象所有权），多个指针指向同一个对象，最后一个指针被销毁时，对象才会被释放
	// weak_ptr：用于辅助 shared_ptr
	// unique_ptr：独占式指针（独占对象所有权），只有一个指针能够指向对象，该对象的所有权可以移交（move）

	// shared_ptr: 管理的资源被多个对象共享，内部采用引用计数跟踪所有者的个数，当最后一个所有者被析构时，资源即被释放。

	// (3.1.2) shared_ptr 基础
	// shared_ptr 有额外开销，适用于多个指针共享一个堆对象
	// shared_ptr 的工作原理是引用计数，只有最后一个指向堆对象的指针被销毁时，对象才会被真正释放
	// 对于最后一个指向堆对象的 shared_ptr 指针，当发生下列两种情况时，会真正释放指向的堆对象
	// a) shared_ptr 指针被析构的时候
	// b) shared_ptr 指向其他对象的时候

	// shared_ptr 的简单使用
	
	// 指向 int 类型内存的智能指针，变量名为 sp，但目前为空，没有指向任何对象
	std::shared_ptr<int> sp;

	// 使用 new 语句构造 shared_ptr
	std::shared_ptr<int> sp1(new int(100));

	// 不能使用等号赋值，因为智能指针不能进行隐式类型转换，只能进行显式类型转换，必须直接构造函数初始化
	// std::shared_ptr<int> sp2 = new int(100);
	
	// 使用显式强制类型转换
	std::shared_ptr<int> sp2 = static_cast<std::shared_ptr<int>>(new int(100));

	// return shared_ptr
	std::shared_ptr<int> sp3 = _nmsp1::make_int(100);

	// 允许用裸指针初始化 shared_ptr 
	// 但穿插使用裸指针和 shared_ptr 容易出错，尽量不要混用裸指针和 shared_ptr
	int* p2 = new int(100);
	std::shared_ptr<int> sp4(p2);

	// (3.1.3) make_shared
	// make_shared 函数模板，能安全高效的创建 shared_ptr 对象
	std::shared_ptr<int> sp5 = std::make_shared<int>(100);
	std::shared_ptr<std::string> sp6 = std::make_shared<std::string>(5, 'a');
	std::shared_ptr<int> sp7 = std::make_shared<int>();
	sp7 = std::make_shared<int>(400);
	auto sp8 = std::make_shared<std::string>(6, 'a');
}

void test3_2() {
	// (3.2.1) shared_ptr 引用计数的增加和减少
	// 引用计数增加
	// 每个 shared_ptr 都会记录有多少个 shared_ptr 同时指向相同的堆对象
	//// 问题1 ？？？？？？？？ 这个地方记录引用计数数目包不包括自己？，是每个 shared_ptr 都包含一个引用计数的变量吗？
	//// 问题2 ？？？？？？？？，引用计数增多时，工作原理是什么
	
	// 什么情况下，所有指向同一个堆对象的 shared_ptr 指针的引用计数都会增加 1
	// a) 对已有智能指针进行拷贝时
	auto sp1 = std::make_shared<int>(100);
	auto sp2 = sp1;

	// b) 将智能指针作为实参值传递发生了对象拷贝时
	_nmsp1::func_value(sp1);
	// 传 shared_ptr 的引用，引用计数不会加 1
	_nmsp1::func_reference(sp1);

	// c) return shared_ptr 发生了拷贝时
	auto sp3 = _nmsp1::func_return_value(sp1);

	// 引用计数减少
	// 什么情况下，所有指向同一个堆对象的 shared_ptr 指针的引用计数都会减少 1
	// a) 给 shared_ptr 赋予新值，让其指向一个新的堆对象
	sp3 = std::make_shared<int>(100);
	sp2 = std::make_shared<int>(200);
	sp1 = std::make_shared<int>(300);

	// b) 局部的 shared_ptr 离开其作用域
	_nmsp1::func_value(sp1);
	
}

void test3_3() {
	// (3.3) shared_ptr 常用操作
	// use_count()：返回有多少个智能指针指向同一个堆对象，主要用于调试目的
	// unique()：判断该智能指针是否独占一个堆对象（如果 shared_ptr 初始化时未指向堆对象，则返回 false)
	std::shared_ptr<int> sp1(new int(100));
	std::cout << "use_count = " << sp1.use_count() << std::endl;
	// 报错：unique is deprecated in C++17.
	// std::cout << "if unique(): " << (sp1.unique() ? "True" : "False") << std::endl;
	auto sp2 = sp1;
	std::cout << "use_count = " << sp1.use_count() << std::endl;
	// 报错：unique is deprecated in C++17.
	// std::cout << "if unique(): " << (sp1.unique() ? "True" : "False") << std::endl;
	auto sp3 = sp2;
	std::cout << "use_count = " << sp1.use_count() << std::endl;
	// 报错：unique is deprecated in C++17.
	// std::cout << "if unique(): " << (sp1.unique() ? "True" : "False") << std::endl;
	std::cout << std::endl;

	// reset()：恢复 / 复位 / 重置
	// a) 当 reset() 不带参数时：
	// 若 sp 是唯一指向该堆对象的指针，那么释放 pi 所指向的堆对象，并将 pi 置空
	// 若 sp 不是唯一指向该堆对象的指针，其他 share_ptr 指针的引用计数减 1，同时将 pi 置空
	std::shared_ptr<int> p1(new int(100));
	auto p2 = p1;
	p2.reset();
	std::cout << "use_count = " << p1.use_count() << std::endl;
	p1.reset();
	std::cout << "use_count = " << p1.use_count() << std::endl;
	std::cout << std::endl;
	// reset()：恢复 / 复位 / 重置
	// a) 当 reset() 不带参数时：
	// 若 sp 是唯一指向该堆对象的指针，那么释放 pi 所指向的堆对象，并将 pi 置空
	// 若 sp 不是唯一指向该堆对象的指针，其他 share_ptr 指针的引用计数减 1，同时将 pi 置空
	std::shared_ptr<int> p3 = std::make_shared<int>(100);
	p3.reset(new int(300));
	std::cout << "*p3 = " << *p3 << std::endl;
	// 空 shared_ptr 对象可以通过 reset() 重新初始化
	std::shared_ptr<int> p4;
	// 未初始化直接调用 *p4 会出现异常
	// std::cout << "*p4 = " << *p4 << std::endl;
	p4.reset(new int(400));
	std::cout << "*p4 = " << *p4 << std::endl;
	std::cout << std::endl;

	// *p 解引用：获得 p 指向的对象
	std::shared_ptr<int> pt1(new int(123456));
	std::cout << "*pt1 = " << *pt1 << std::endl;
	std::cout << std::endl;

	// get()：返回智能指针对象中所保存的裸指针
	// 需要小心使用，如果智能指针调用 reset() 释放了所指向的堆对象，那么这个返回的裸指针也就没有意义了
	// get() 的使用是不安全的，但标准库实现 get() 函数是考虑到有些函数只能传递内置的裸指针，比如 Linux 下的系统调用，第三方库等
	// 这也提示了程序员，虽然智能指针帮助简化了内存管理，但智能指针也不是万能的，还是需要对内存管理有深厚的理解和经验
	std::shared_ptr<int> pt2(new int(654321));
	std::cout << "*pt2 = " << *pt2 << std::endl;
	int* p = pt2.get();
	*p = 45;
	std::cout << "*pt2 = " << *pt2 << std::endl;
	std::cout << std::endl;

	// swap()：用于交换 2 个智能指针所指向的堆对象，一般不常用
	std::shared_ptr<std::string> ps1(new std::string("123"));
	std::shared_ptr<std::string> ps2(new std::string("abc"));
	std::cout << "*ps1 = " << *ps1 << std::endl;
	std::cout << "*ps2 = " << *ps2 << std::endl;
	// ps1.swap(ps2);
	std::swap(ps1, ps2);
	std::cout << " After Swap ps1 and ps2!" << std::endl;
	std::cout << "*ps1 = " << *ps1 << std::endl;
	std::cout << "*ps2 = " << *ps2 << std::endl;
	std::cout << std::endl;

	// = nullptr
	// a) 将所指向的堆对象引用计数减一，若引用计数变为 0 了，则释放智能指针所指向的对象
	// b) 将智能指针置空
	std::shared_ptr<std::string> ps3(new std::string("nullptr"));
	auto ps4 = ps3;
	ps4 = nullptr;
	ps3 = nullptr;
	std::cout << std::endl;

	// 智能指针名字作为判断条件
	std::shared_ptr<std::string> ps5(new std::string("Hello!"));
	if (ps5) std::cout << "*ps5 = " << *ps5 << std::endl;
	ps5 = nullptr;
	if (!ps5) std::cout << "*ps5 is empty!" << std::endl;
	std::cout << std::endl;
}

void test3_4() {
	// (3.4.1) 指定删除器
	// 智能指针能够在合适的时机删除所指向的堆对象
	// 缺省情况下，智能指针将 delete 作为默认的资源析构方式
	// 如果希望做一些额外的处理，例如打印日志等，可以给智能指针指定删除器以达到目的
	// 指定删除器的方法一般是在参数中添加删除器的函数名
	std::shared_ptr<int> pi(new int(123456), _nmsp2::deleter);
	pi.reset();
	std::cout << std::endl;

	// 使用 lambda 表达式作为删除器
	std::shared_ptr<int> pi1(new int(123456), [](int* p)->void {
		std::cout << "整型 lambda deleter 执行！" << std::endl;
		// 如果选择取代系统缺省的删除器，则有义务自己删除对象
		delete p;
	});
	pi1.reset();
	std::cout << std::endl;

	// (3.4.2) share_ptr 管理动态数组的问题
	// 有些情况默认删除器处理不了，需要提供自定义删除器
	// 一个典型的例子是用 share_ptr 管理动态数组时
	// share_ptr 管理整形动态数组
	std::shared_ptr<int> pi2(new int[10]{ 0 }, [](int* p)->void {
		std::cout << "整型数组 lambda deleter 执行！" << std::endl;
		delete[] p;
	});
	pi2.reset();
	std::cout << std::endl;

	// share_ptr 管理类类型动态数组
	std::shared_ptr<_nmsp1::A> pa(new _nmsp1::A[10](), [](_nmsp1::A* p)->void {
		std::cout << "类类型数组 lambda deleter 执行！" << std::endl;
		delete[] p;
	});
	pa.reset();
	std::cout << std::endl;
	
	// 使用 default_delete 来做删除器（default_delete 是标准库中的模板类）
	std::shared_ptr<int> pi3(new int[10]{ 0 }, std::default_delete<int[]>());
	pi3.reset();
	std::cout << std::endl;
	
	std::shared_ptr<_nmsp1::A> pa1(new _nmsp1::A[10](), std::default_delete<_nmsp1::A[]>());
	pa1.reset();
	std::cout << std::endl;

	// C++17 标准支持的新语法
	std::shared_ptr<int[]> pi4(new int[10]{ 0 });
	pi4.reset();
	std::cout << std::endl;

	std::shared_ptr<_nmsp1::A[]> pa2(new _nmsp1::A[10]());
	pa2.reset();
	std::cout << std::endl;

	// shared_ptr 使用下标访问符
	std::shared_ptr<int[]> p(new int[10]{ 0 });
	for (int i = 0; i < 10; ++i) {
		p[i] = i + 1;
		std::cout << p[i];
	}

	// 使用封装的 shared_ptr 数组
	auto array_p1 = _nmsp2::make_shared_array<int>(10);
	auto array_p2 = _nmsp2::make_shared_array<_nmsp1::A>(10);

	// 指定删除器额外说明
	// 即使 2 个 shared_ptr 指定了不同的删除器，只要它们所指向的对象类型相同
	// 那么这两个 shared_ptr 属于同一类型
	auto lambda1 = [](int* p)->void {
		std::cout << "lambad1 执行！\n";
		delete p;
	};
	auto lambda2 = [](int* p)->void {
		std::cout << "lambad2 执行！\n";
		delete p;
	};
	
	// 在创建时决定这段内存由 lambda1 释放，不会由于赋值等原因导致更换删除器
	std::shared_ptr<int> sp1(new int(100), lambda1);
	std::shared_ptr<int> sp2(new int(200), lambda2);
	// sp2 会先调用 lambda2 把自己所执行的堆对象释放掉，然后指向 sp1 所指向的堆对象, sp1 的引用计数为 2
	// 最终 test3_4() 执行结束，会调用 lambda1 来释放 sp1 所指向的堆对象
	// 这行代码也表示 sp1 和 sp2 属于同一个类型，所以才能将 sp1 赋值给 sp2
	sp2 = sp1;

	// 类型相同，意味着可以将 sp1 和 sp2 放入相应元素类型的容器中
	std::vector<std::shared_ptr<int>> v{ std::move(sp1), sp2 };

	// 需要注意，make_shared 不能指定删除器
}

void test4_1() {
	// weak_ptr: 与 shared_ptr 配合使用，虽然能访问资源但却不享有资源的所有权，不影响资源的引用计数
	// 有可能资源已被释放，但 weak_ptr 仍然存在，因此每次访问资源时都需要判断资源是否有效


	// (4.1.1) weak_ptr 概述（弱指针，弱共享，弱引用）
	// weak_ptr 用于辅助 shared_ptr，表示弱引用指针，与之相对的 shared_ptr 表示强引用指针
	// weak_ptr 用于指向一个由 shared_ptr 管理的堆对象，但 weak_ptr 不参与控制指向堆对象的生命周期
	// 换而言之，当 weak_ptr 指向一个由 shared_ptr 管理的堆对象时，不会影响其引用计数
	// 也就是说，weak_ptr 的构造和析构不会增加或减少其指向对象的引用计数
	// 当堆对象的引用计数变为 0 时，无论是否有 weak_ptr 指向该对象，都不会影响 shared_ptr 正常释放该对象
	/////// 由此可能导致 weak_ptr 指向的对象已经被 shared_ptr 析构了，导致 weak_ptr 使用对象发生错误
	// weak_ptr 可以解为用于监视 shared_ptr 指向对象的生命周期，是对 shared_ptr 能力的扩充
	// weak_ptr 不是一种独立的智能指针，不能用来操作所指向的资源（类似一个旁观者）
	// weak_ptr 能够监视它所指向的对象是否存在

	// (4.1.2) 创键 weak_ptr
	auto pi = std::make_shared<int>(100);
	// 此处允许隐式类型转换
	std::weak_ptr<int> pi1 = pi;
	std::weak_ptr<int> pi2 = pi1;
	
	// 由于 weak_ptr 所指向的对象可能被释放，所以不能使用 weak_ptr 直接访问对象，而必须借助 lock() 函数访问
	// lock()：用于检查 weak_ptr 所指向的对象是否存在
	// 如果存在，则返回一个指向该对象的 shared_ptr，同时指向该对象的引用计数加 1
	// 如果不存在，则返回一个空的 shared_ptr
	auto sp = std::make_shared<std::string>("Hello!");
	std::weak_ptr<std::string> sp1(sp);
	auto sp2 = sp1.lock();
	if (sp2) std::cout << *sp2 << std::endl;
	sp.reset();
	sp2.reset();
	// sp1 监视到所指向的对象已经不存在了
	auto sp4 = sp1.lock();
	if (!sp4) std::cout << "Not Exist!" << std::endl;
}

void test4_2() {
	// (4.2) weak_ptr 常用操作	

	// use_count()：获取该弱指针共享对象的其他 shard_ptr 的数量（强引用的数量）
	auto ps = std::make_shared<int>(100);
	auto ps1 = ps;
	std::weak_ptr<int> pw = ps;
	std::cout << "pw.use_count(): "<<pw.use_count() << std::endl;

	 // expired()：判断 weak_ptr 指向的堆对象是否过期（强引用计数是否变为 0）
	std::cout << "pw.expired(): " << (pw.expired() ? "True" : "False") << std::endl;
	ps.reset();
	ps1.reset();
	std::cout << "pw.expired(): " << (pw.expired() ? "True" : "False") << std::endl;

	// reset()：将 weak_ptr 置空
	// 不会影响指向该对象的强引用数量，但指向该对象的弱引用数量减一
	ps = std::make_shared<int>(100);
	pw = ps;
	auto pw1 = pw;

	// lock()：用于检查 weak_ptr 所指向的对象是否存在
	// 如果存在，则返回一个指向该对象的 shared_ptr，同时指向该对象的引用计数加 1
	// 如果不存在，则返回一个空的 shared_ptr
	
	auto ps2 = std::make_shared<int>(100);
	std::cout << "ps2: " << *ps2 << std::endl;
	std::weak_ptr<int> pw2 = ps2;
	// ps2.reset();
	if (!pw2.expired()) {
		auto tmp = pw2.lock();
		if (tmp) *tmp = 200;
		std::cout << "ps2: " << *ps2 << std::endl;
	}else {
		std::cout << "ps2 expired!" << std::endl;
	}
	
	// weak_ptr 的尺寸问题
	// weak_ptr 的尺寸和 shared_ptr 尺寸一样大
	int* p3;
	std::weak_ptr<int> pw3;
	// 4 字节（裸指针的大小）
	std::cout << "sizeof(p3) = " << sizeof(p3) << std::endl;
	// 8 字节（包含 2 个裸指针）
	// 第一个指针指向堆内存上分配的对象， 第二个指针指向该对象控制块的首地址
	// 控制块实际上是在创建第一个 shared_ptr 时创建的，创建第一个 weak_ptr 时只是指向它，并不创建它
	// 控制块中包含了指向该对象的强引用计数，弱引用计数以及其他数据（如自定义删除器等）
	std::cout << "sizeof(pw3) = " << sizeof(pw3) << std::endl;
}

void test5_1() {
	// (5.1) 慎用裸指针和 get() 函数
	// a) 慎用裸指针
	int* p = new int(100);
	// 不能进行隐式类型转换
	// _nmsp2::func(p);
	_nmsp2::func(std::shared_ptr<int>(p));
	// 此处 p 指向的内存空间在传递进入 func() 后被释放了，导致不能正常使用 p 指针
	// *p = 200;

	// 正确用法
	// 当用裸指针初始化智能指针后，内存管理权限移交给了 shared_ptr 对象
	// 在后续代码中尽量不要再使用裸指针，以免访问被释放的内存
	int* p1 = new int(200);
	std::shared_ptr<int> sp1(p1);
	_nmsp2::func(sp1);
	*p1 = 300;

	// b) 绝对不能用同一个裸指针初始化多个 shared_ptr 对象
	// 如果重复初始化，会导致这些 shared_ptr 对象析构时多次释放同一块内存，发生异常
	// int *p2 = new int(300);
	// std::shared_ptr<int> sp2(p2);
	// std::shared_ptr<int> sp3(p2);
	
	// c) 慎用 get() 返回指针
	// 1) 使用 get() 返回裸指针 p 时，有可能使用被释放的内存
	// 比如在使用 p 时，所有 shared_ptr 生命期都结束，则会出错
	// int* p3 = nullptr;
	// {
	//	 std::shared_ptr<int> sp4(new int(100));
	// 	 p3 = sp4.get();
	// }
	// *p3 = 200;
	
	
	// 2) 使用 get() 返回裸指针 p 时，不能堆 p 使用 delete
	// 因为内存实际由 shared_ptr 管理，不应该自己管理
	// std::shared_ptr<int> sp5(new int(100));
	// int* p4 = sp5.get();
	// delete p4;

	// 3) 不能用 get() 返回的裸指针去初始化其他智能指针
	std::shared_ptr<int> sp6(new int (100));
	{
		std::shared_ptr<int> sp7(sp6.get());
	}
	// 这行代码对释放的内存赋值，却完全不会报错
	// 相当于非法使用了不属于本程序的堆内存，这会导致不可预料的后果
	// 此处 delete 后的裸指针都不能赋值，而智能指针却可以，是一个非常隐晦的陷阱
	*sp6 = 5;
	// 甚至能够正常打印出来
	std::cout << *sp6 << std::endl;
	
	//// 测试对释放内存后的普通指针赋值
	//int* p5 = new int(200);
	//delete p5;
	//// 发生写访问权限冲突的异常
	//*p5 = 100;
	//std::cout << *p5 << std::endl;
}

void test5_2() {
	// (5.2.1) 不要把类对象指针 this 作为 shared_ptr 返回
	std::shared_ptr<_nmsp3::A> pa1(new _nmsp3::A());
	// 相当于用指向对象的裸指针去初始化一个智能指针，导致一块内存多次释放
	// std::shared_ptr<_nmsp3::A> pa2 = pa1->get_self();
	// std::shared_ptr<_nmsp3::A> pa3(pa1->get_this());

	// 使用 enable_shared_from_this 类
	std::shared_ptr<_nmsp3::SA> pa4(new _nmsp3::SA());
	std::shared_ptr<_nmsp3::SA> pa5 = pa4->get_self();
	// enable_shared_from_this 有一个 weak_ptr 能够监视 this
	// 在调用 shared_from_this 这个方法时，实际上内部调用了 weak_ptr 的lock()方法 （存疑？）
	// 想不太到实现的原理， 
	
	// 思考：能用一个裸指针初始化 weak_ptr 吗？
	// 实践证明不能
	//_nmsp3::SA* p1 = new _nmsp3::SA();
	//std::weak_ptr<_nmsp3::SA> pw1;
	//pw1.reset(p1);

	// (5.2.2) 避免循环引用
	//std::shared_ptr<_nmsp3::CA> pca(new _nmsp3::CA());
	//std::shared_ptr<_nmsp3::CB> pcb(new _nmsp3::CB());
	//// 循环引用导致 2 个对象都没释放（为什么？）
	//// 下面思考原因
	//// 由于 pcb 先析构，析构时仅仅减少了 CB 对象的引用计数，并未析构内存上的 CB 对象
	//// CB 对象等待 CA 的成员函数 m_b 析构（即 CB 的引用计数减为 0）时，再真正释放内存
	//// 当 pca 析构时，也只是将 CA 对象的引用计数减一，并未真正释放内存
	//// 而后，CA 对象等待 CB 的成员函数 m_a 析构（即 CA 的引用计数减为 0）时，再真正释放内存
	//// 由此 CA，CB 相互都在等对方的指向自己的 shared_ptr 成员变量析构，最终两者都不能析构，导致内存泄露
	//pca->m_b = pcb;
	//pcb->m_a = pca;

	// 解决方法是将 CA，CB 中任何一个 shared_ptr 改成 weak_ptr
	// 将其中一个类的 share_ptr 改为 weak_ptr 会使得这个类被真正释放，相当于打破了相互的等待
	// 只要能保证原本 share_ptr 的生命周期，weak_ptr 完全能够当作 shared_ptr 使用，不过性能要差一些
	std::shared_ptr<_nmsp3::CA> pca(new _nmsp3::CA());
	std::shared_ptr<_nmsp3::CB> pcb(new _nmsp3::CB());
	pca->m_b = pcb;
	pcb->m_a = pca;
}

void test5_3(){
	// (5.3.1) 性能说明
	// shared_ptr 占用空间是普通裸指针的 2 倍大小
	int* p;
	std::shared_ptr<int> ps;
	// 4 字节（裸指针的大小）
	std::cout << "sizeof(p3) = " << sizeof(p) << std::endl;
	// 8 字节（包含 2 个裸指针）
	// 第一个指针指向堆内存上分配的对象， 第二个指针指向该对象控制块的首地址
	// 控制块实际上是在创建第一个指向堆对象的 shared_ptr 时创建，（此后复制其他 shared_ptr 都不会在创建了？？？ 我猜测的，并不确定）
	// 控制块中包含了指向该对象的强引用计数，弱引用计数以及其他数据（如自定义删除器，分配器等）
	// 由于 weak_ptr 也指向 share_ptr 创建的控制块，所以 weak_ptr 必须依赖于 share_ptr 而存在 
	std::cout << "sizeof(pw3) = " << sizeof(ps) << std::endl;

	// 控制块创建的时机
	// a) make_shared() 分配并初始化一个对象，并返回指向该对象的 shared_ptr
	// 所以 make_shared() 总是会创建一个新的控制块

	// b) 用 new 或裸指针初始化一个新的 shared_ptr 时
	// 如果使用同一个裸指针初始化多个 share_ptr，则会创建多个控制块，并多次释放同一块内存，产生异常
	
	// (5.3.2) 移动语义
	std::shared_ptr<int> sp1(new int(100));
	// 移动构造 sp2，sp1 不会再指向原对象，引用计数仍为 1
	// 相当于 sp1 将堆对象的管理控制权移交（而不是共享）给了 sp2
	std::shared_ptr<int> sp2 = std::move(sp2);
	
	// (5.3.3) 补充说明与使用建议
	// a) 可以为智能指针提供分配器
	// b) 对待内存依然要小心谨慎
	// c) 优先考虑使用 make_shared() 效率更高
	// 据说可能分配 2 次内存，1 次为 string 分配，1 次为控制块分配
	std::shared_ptr<std::string> ps1(new std::string("abc"));
	// 据说只用分配 1 次内存，一次性为 string 和控制块同时分配一块大内存
	std::shared_ptr<std::string> ps2 = std::make_shared<std::string>("abc");
}

void test6_1() {
	// (6.1.1) unique_ptr 概述与常用操作(1)
	// unique_ptr 是独占式（专属所有权）智能指针
	// 也就是说同一时刻，只能有一个 unique_ptr 指向堆对象
	
	// (6.1.2) unique_ptr 的初始化
	// 初始化一个可以指向 int 对象的空智能指针
	std::unique_ptr<int> pi;
	if (!pi)std::cout << "pi is empty!" << std::endl;
	std::unique_ptr<int> pi1(new int(100));
	if (!pi)std::cout << "*pi1 = " <<*pi1<< std::endl;

	// make_unique() (C++14)
	// make_unique 与 make_shared 一样，不支持指定删除器
	// 建议优先使用 make_unique，性能更高（内置类型可能不占优势，但类类型占优势，效率主要高在内存分配方面）
	std::unique_ptr<std::string> ps = std::make_unique<std::string>("Hello!");

	// (6.1.3) unique_ptr 常用操作(1)
	// a) unique_ptr 不支持拷贝构造函数和赋值运算符
	// 由于独占对象，所以 unique_ptr 不支持拷贝构造函数和赋值运算符，但支持 move 语义
	std::unique_ptr<int> pi2(new int(100));
	// 报错：使用已删除的函数
	// std::unique_ptr<int> pi3(pi2);
	// 报错：使用已删除的函数
	// std::unique_ptr<int> pi4 = pi2;
	std::unique_ptr<int> pi5 = std::move(pi2);
			
	// b) realse()：放弃对堆对象的控制权，将智能指针置空，并返回裸指针
	// 对于返回的裸指针，可以手动 delete，也可用用其初始化另一个智能指针
	std::unique_ptr<int> up1(new int(0));
	std::unique_ptr<int> up2(up1.release());
	if (!up1)std::cout << "up1 is empty!" << std::endl;
	// 注意：只调用 release() 而不去接收并 delete 返回的裸指针会导致内存泄露
	// up2.release();
	std::unique_ptr<int> up3;
	auto p1 = up1.release();
	auto p2 = up3.release();
	// 通过验证，对于初始化为空或 release() 后的 unique_ptr，其内部的裸指针都置为了 nullptr
	if (p1) std::cout << "p1 指向未知内存!" << std::endl;
	else std::cout << "p1 is nullptr!" << std::endl;
	if (p2) std::cout << "p2 is 指向未知内存！" << std::endl;
	else std::cout << "p2 is nullptr!" << std::endl;
	// 通过验证，delete 后的裸指针指向未知内存，在大多数编译器实现下，是非法指向原来位置的内存
	int* p3 = new int(300);
	delete p3;
	if (p3) std::cout << "p3 is 指向未知内存！" << std::endl;
	else std::cout << "p3 is nullptr!" << std::endl;

	// c) reset()：恢复 / 复位 / 重置
	// 1) 当 reset() 不带参数时：释放智能指针所指向的对象，并将智能指针置空
	std::unique_ptr<int> up4(new int(400));
	up4.reset();
	// 2) 当 reset() 带参数时：释放智能指针所指向的对象，并让该智能指针指向新传入的对象
	std::unique_ptr<int> up5(new int(500));
	std::unique_ptr<int> up6(new int(600));
	up6.reset(up5.release());
	std::cout << "*up6 = " << *up6 << std::endl;
	up6.reset(new int(123456));
	std::cout << "*up6 = " << *up6 << std::endl;
}

void test6_2() {
	// (6.2.1) unique_ptr 常用操作(2)
	// a) = nullptr：释放智能指针所指向的对象，并将智能指针置空
	std::unique_ptr<int> p1(new int(100));
	p1 = nullptr;

	// b) 指向一个数组
	// 实测下来，内置类型如 int，float 等模板变量类型可以使用 int 或 int[]
	// 这是由于实际上可以使用 delete 删除基本类型动态数组，不过不推荐如此做，最好使用 int[] 作为模板类型
	std::unique_ptr<int[]> p2(new int[10]);
	for (int i = 0; i < 10; ++i) {
		p2[i] = i;
		std::cout << p2[i];
	}
	std::cout << std::endl;
	std::cout << std::endl;
	// 注意，此处数组使用可以越界，这是因为智能指针本身并不检查下标是否越界
	// 实测下来，下标越界访问非法内存，会导致系统严重的长时间卡顿并产生异常
	//for (int i = 0; i < 15; ++i) {
	//	p2[i] = i;
	//	std::cout << p2[i];
	//}
	//std::cout << std::endl;
	// 实测下来，当自定义类类型没有实现析构函数而使用默认的析构函数时，可以用 T 类型作为模板参数
	// 原因也是也是可以调用 delete 删除动态数组，不过依然不推荐如此做，最好使用 T[] 作为智能指针的模板参数
	std::unique_ptr <_nmsp1::A[]> p3(new _nmsp1::A[10]);
	// 总结：一律使用带 [] 的类型作为动态数组智能指针的模板参数
	std::cout << std::endl;
	
	// c) get()：返回智能指针所保存的裸指针
	// d) 地址解析符 *：获取智能指针指向的对象并直接操作
	std::unique_ptr <std::string> p4(new std::string("abc"));
	std::string* p5 = p4.get();
	// 通过 p5 更改 p4;
	*p5 = "Hello!";
	std::cout << "p4 = " << *p4 << std::endl;
	std::cout << std::endl;

	// e) swap()：交换两个智能指针所指向的对象
	std::unique_ptr<std::string> up1(new std::string("123"));
	std::unique_ptr<std::string> up2(new std::string("abc"));
	std::cout << "*up1 = " << *up1 << std::endl;
	std::cout << "*up2 = " << *up2 << std::endl;
	up1.swap(up2);
	// std::swap(up1, up2);
	std::cout << " After Swap up1 and up2!" << std::endl;
	std::cout << "*ps1 = " << *up1 << std::endl;
	std::cout << "*ps2 = " << *up2 << std::endl;
	std::cout << std::endl;

	// f) 智能指针名作为判断条件
	std::unique_ptr<int> up3(new int(300));
	if (up3) std::cout << "*up3 = " << *up3 << std::endl;
	up3.reset();
	if (!up3) std::cout << "up3 is empty!" << std::endl;
	std::cout << std::endl;

	// g) unique_ptr 转换成 shared_ptr 类型
	// 如果 unique_ptr 为右值，可将其赋值给 shared_ptr，shared_ptr 会接管 unique_ptr 拥有的对象
	// 因为 unique_ptr 代表独占，如果将其拷贝给 shared_ptr 就破坏了其独占性
	// 所以只能通过 move 语义将其管理的对象移交给 shared_ptr
	// 而右值正代表临时值或将亡值，正好用于将对象移交给 share_ptr
	// 此处会创建一个控制块
	std::shared_ptr<std::string> sp = _nmsp3::func();
	std::unique_ptr<std::string> up4(new std::string("Hello!"));
	// 使用 move 语义将左值变量转为右值
	std::shared_ptr<std::string> sp2 = std::move(up4);
}

void test6_3() {
	// (6.3.1) 函数返回 uniqu_ptr (返回将亡值）
	// 此时是否发生了拷贝（存疑？）
	auto up = _nmsp3::return_unique();

	// (6.3.2) 指定删除器（可调用对象）
	// unique_ptr 格式：unique_ptr<T, deleter> var(arg, deleter);
	// shared_ptr 格式：shared_ptr<T> variable(arg, deleter);
	typedef void(*del1)(std::string*&);
	std::unique_ptr<std::string, del1> up1(new std::string("Hello!"), _nmsp4::deleter_function);
	using del2 = void(*)(std::string*&);
	std::unique_ptr<std::string, del2> up2(new std::string("Hello!"), _nmsp4::deleter_function);
	// 由于decltype 返回的是函数类型，所以必须加 *
	typedef decltype(_nmsp4::deleter_function)* del3;
	std::unique_ptr<std::string, del3> up3(new std::string("Hello!"), _nmsp4::deleter_function);
	std::unique_ptr<std::string, decltype(_nmsp4::deleter_function)*> up4(new std::string("Hello!"), _nmsp4::deleter_function);
	auto deleter_lambda = [](std::string*& p)->void {
		std::cout << "deleter_lambda() 执行！" << std::endl;
		delete p;
		p = nullptr;
	};
	std::unique_ptr<std::string, decltype(deleter_lambda)> up5(new std::string("Hello!"), deleter_lambda);
	std::unique_ptr<std::string, _nmsp4::deleter_class> up6(new std::string("Hello!"), _nmsp4::deleter_class());

	// 对于两个 shared_ptr，即使给它们指定不同类型的删除器，只要它们所指向对象的类型相同，则它们属于同一类型，相互之间可以拷贝、赋值或移交所有权（move)
	// 但对于 nuique_ptr 不同， 由于在变量声明或定义时，向 nuique_ptr 的模板参数中传入了删除器的类型，所以删除器类型也属于 nuique_ptr 类型的一部分
	// 这意味着对于所指向的对象类型相同但删除器类型不同的 unique_ptr， 它们是属于不同类型的，相互之间不能移交所有权(move)
	// up4, up5 类型相同，所以 move 合法
	up4 = std::move(up5);
	// up5, up6 类型不同，所以 move 不合法
	// up5 = std::move(up6);
	
	// (6.3.3) 尺寸问题
	int* p;
	// 一般情况下，unique_ptr 占用的空间大小是 4 字节
	std::unique_ptr<std::string> ps = std::make_unique<std::string>("Hello!");
	std::cout << "sizeof(p) = " << sizeof(p) << std::endl;
	std::cout << "sizeof(ps) = " << sizeof(ps) << std::endl;

	// 如果增加了删除器，则空间有可能增大，对效率会产生影响
	// 用函数指针作为删除器占用空间变成了 8 字节
	std::cout << "function： sizeof(up4) = " << sizeof(up4) << std::endl;
	// 保持 4 字节
	std::cout << "lambda: sizeof(up5) = " << sizeof(up5) << std::endl;
	// 保持 4 字节
	std::cout << "class: sizeof(up6) = " << sizeof(up6) << std::endl;
	
	// (6.3.4) 总结
	// 1) 智能指针的主要目的是帮助释放内存，防止忘记释放内存导致内存泄露，即帮助释放可能忘记释放的内存（也是 RALL 的思想）
	// 2) auto_ptr 是 C++98 时代的智能指针，具有 unique_ptr 的一部分特性，以下是它被废弃的原因如下
	// a) 不能在容器中保存
	// b) 不能从函数中（作为将亡值）返回
	// c) 支持拷贝构造函数或赋值运算符，但其实现类似于 move 语义，会造成赋值对象不可再使用
	// 总结：auto_ptr 由于设计问题，容易误用，导致潜在的程序崩溃等问题，所以 C++11 后被弃用了
	// 如果程序要求用多个指向同一个对象的指针，应该首选 shared_ptr;
	// 如果程序不需要多个指向同一个对象的指针，应该首选 unique_ptr;
}

int main() {
	// 一、直接使用 new / delete 管理内存	
	// test1();

	// 二、new / delete 探秘
	// test2();
	
	// 三、智能指针与 shared_ptr
	// (3.1) 智能指针概述与 shared_ptr 基础
	// test3_1();

	// (3.2) shared_ptr 引用计数的增加和减少
	// test3_2();

	// (3.3) shared_ptr 常用操作
	// test3_3();

	// (3.4) 指定删除器以及 share_ptr 管理动态数组的问题
	// test3_4();

	// 四、weak_ptr
	// (4.1) weak_ptr 基础
	// test4_1();

	// (4.2) weak_ptr 常用操作与尺寸问题
	// test4_2();

	// 五、shared_ptr 陷阱分析
	// (5.1) 慎用裸指针与 get() 函数
	// test5_1();
	
	// (5.2) 对象指针 this 与 循环引用
	// test5_2();

	// (5.3) 性能说明与 move 语义
	// test5_3();

	// 六、unique_ptr 
	// (6.1) unique_ptr 概述与常用操作(1)
	// test6_1();

	// (6.2) unique_ptr 常用操作(2)
	// test6_2();

	// (6.3) 
	test6_3();

	return 0;
}

