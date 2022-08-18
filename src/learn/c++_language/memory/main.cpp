#include <iostream>

namespace _nmsp1 {
	class A {
	public:

	};

	class B {
	public:
		int m_i;
	};

	class C {
	public:
		C(){}
		int m_i;
	};

	class D {
	public:
		D() :m_i(100) {}
		int m_i;
	};
}

namespace _nmsp2 {

}

void test1_1() {
	// (1.1) new 类对象时加不加括号的差别
	// (1.1.1) 对于类类型
	// a) 如果是一个空类，下面两种写法没有区别，不过现实中不可能只写一个空类
	_nmsp1::A* pa1 = new _nmsp1::A;
	_nmsp1::A* pa2 = new _nmsp1::A();
	// b) 如果类中有成员变量但不写构造函数，则带 () 的初始话会把一些和成员变量有关的内存清零，但不是整个对象的内存全部清零
	// 某些情况下，编译器会帮助生成默认构造函数，默认构造函数会将变量的值初始化为零，如果没有生成默认构造函数，则变量初始值是随机的
	_nmsp1::B* pb1 = new _nmsp1::B;
	_nmsp1::B* pb2 = new _nmsp1::B();
	// c) 如果类中有构造函数，则变量的初始化工作会交给构造函数，如果构造函数不主动初始化变量，则变量的初始值是随机的
	_nmsp1::C* pc1 = new _nmsp1::C;
	_nmsp1::C* pc2 = new _nmsp1::C();
	// d) 如果类中有构造函数，则变量的初始化工作会交给构造函数，且两种调用的效果是一样的
	_nmsp1::D* pd1 = new _nmsp1::D;
	_nmsp1::D* pd2 = new _nmsp1::D();

	// (1.1.2) 对于简单类型
	int* p = new int;			// 初值随机
	int* p1 = new int();		// 初值为 0
	int* p2 = new int(100);		// 初值随机
}

void test1_2() {
	// (1.2) new 与 delete 的基本工作原理
	// new 本质上是一个关键字/操作符，而不是一个函数
	// new 先后分别调用了 operator new() 和构造函数 A::A()
	// 而 operator new 是一个真正的函数，其内部调用了 malloc()
	_nmsp1::A* pa = new _nmsp1::A;

	// delete 本质上是一个关键字/操作符，而不是一个函数
	// delete 先后分别调用了析构函数A::~A()和 operator delete()
	// 而 operator delete() 是一个真正的函数，其内部调用了 free()
	delete pa;
}

int main() {
	// 一、概述与回顾
	// (1.1) new T 与 new T() 的区别
 	// test1_1();

	// (1.2) new 与 delete 的基本工作原理
	test1_2();
	
	return 0;
}